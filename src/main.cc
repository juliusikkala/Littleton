#include "window.hh"
#include "resources.hh"
#include "texture.hh"
#include "object.hh"
#include "pipeline.hh"
#include "framebuffer.hh"
#include "method/clear.hh"
#include "method/forward_pass.hh"
#include "method/geometry_pass.hh"
#include "method/lighting_pass.hh"
#include "method/blit_framebuffer.hh"
#include "method/visualize_gbuffer.hh"
#include "method/tonemap.hh"
#include "method/sky.hh"
#include "method/render_shadow_maps.hh"
#include "method/draw_texture.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include "doublebuffer.hh"
#include "shader_store.hh"
#include <iostream>
#include <algorithm>
#include <glm/gtc/random.hpp>

struct deferred_data
{
    deferred_data(
        window& w,
        glm::uvec2 resolution,
        shader_store& shaders,
        render_scene* main_scene
    ):screen(w, resolution, GL_RGB, GL_RGB16F, GL_FLOAT),
      buf(w, resolution),
      sm(shaders, main_scene),
      clear_buf(buf),
      clear_screen(screen.input(0)),
      gp(buf, shaders, main_scene),
      lp(screen.input(0), buf, shaders, main_scene),
      sky(screen.input(0), shaders, main_scene, &buf.get_depth_stencil()),
      tm(screen.input(1), screen.output(1), shaders),
      screen_to_window(w, screen.input(1), method::blit_framebuffer::COLOR_ONLY)
    {
        screen.set_depth_stencil(0, &buf.get_depth_stencil());
    }

    const std::vector<pipeline_method*> get_methods()
    {
        return {
            &sm,
            &clear_buf,
            &clear_screen,
            &gp,
            &lp,
            &sky,
            &tm,
            &screen_to_window
        };
    }

    doublebuffer screen;
    gbuffer buf;

    method::render_shadow_maps sm;
    method::clear clear_buf;
    method::clear clear_screen;
    method::geometry_pass gp;
    method::lighting_pass lp;
    method::sky sky;
    method::tonemap tm;
    method::blit_framebuffer screen_to_window;
};

struct visualizer_data
{
    visualizer_data(
        window& w,
        glm::uvec2 resolution,
        shader_store& shaders,
        render_scene* main_scene
    ):screen(w, resolution, GL_RGB, GL_RGB16F, GL_FLOAT),
      buf(w, resolution),
      clear_buf(buf),
      clear_screen(screen.input(0)),
      gp(buf, shaders, main_scene),
      visualizer(screen.input(0), buf, shaders, main_scene),
      screen_to_window(w, screen.input(0), method::blit_framebuffer::COLOR_ONLY)
    {}

    const std::vector<pipeline_method*> get_methods()
    {
        return {&clear_buf, &clear_screen, &gp, &visualizer, &screen_to_window};
    }

    doublebuffer screen;
    gbuffer buf;

    method::clear clear_buf;
    method::clear clear_screen;
    method::geometry_pass gp;
    method::visualize_gbuffer visualizer;
    method::blit_framebuffer screen_to_window;
};

struct forward_data
{
    forward_data(
        window& w,
        glm::uvec2 resolution,
        shader_store& shaders,
        render_scene* main_scene
    ):color_buffer(w, resolution, GL_RGB, GL_RGB16F, GL_FLOAT),
      depth_buffer(
        w,
        resolution,
        GL_DEPTH_STENCIL,
        GL_DEPTH24_STENCIL8,
        GL_UNSIGNED_INT_24_8,
        texture::DEPTH_PARAMS
      ),
      screen(w, resolution, {&color_buffer}, &depth_buffer),
      postprocess(w, resolution, GL_RGB, GL_RGB16F, GL_FLOAT),
      sm(shaders, main_scene),
      clear_screen(screen),
      fp(screen, shaders, main_scene),
      sky(screen, shaders, main_scene, &depth_buffer),
      tm(postprocess.input(0), color_buffer, shaders),
      postprocess_to_window(
        w,
        postprocess.input(0),
        method::blit_framebuffer::COLOR_ONLY)
    {}


    const std::vector<pipeline_method*> get_methods()
    {
        return {&sm, &clear_screen, &fp, &sky, &tm, &postprocess_to_window};
    }

    texture color_buffer;
    texture depth_buffer;
    framebuffer screen;
    doublebuffer postprocess;

    method::render_shadow_maps sm;
    method::clear clear_screen;
    method::forward_pass fp;
    method::sky sky;
    method::tonemap tm;
    method::blit_framebuffer postprocess_to_window;
};

template<typename T>
class custom_pipeline: public T, public pipeline
{
public:
    custom_pipeline(
        window& w,
        glm::uvec2 resolution,
        shader_store& shaders,
        render_scene* main_scene
    ): T(w, resolution, shaders, main_scene), pipeline(T::get_methods()) {}
};

using deferred_pipeline = custom_pipeline<deferred_data>;
using visualizer_pipeline = custom_pipeline<visualizer_data>;
using forward_pipeline = custom_pipeline<forward_data>;

int main(int argc, char** argv)
{ 
    window w({"dflowers", {1280, 720}, true, true, false});
    w.set_framerate_limit(120);
    std::cout << "GPU Vendor: " << w.get_vendor_name() << std::endl
              << "Renderer:   " << w.get_renderer() << std::endl;

    w.grab_mouse();
    resource_store resources(w);
    resources.add_dfo("data/test_scene.dfo", "data");
    resources.add_dfo("data/earth.dfo", "data");

    shader_store shaders(w, {"data/shaders/"});

    object* suzanne = resources.get<object>("Suzanne");
    object* earth = resources.get<object>("Earth");
    earth->set_position(glm::vec3(0, 2, -6));
    earth->scale(2);

    camera cam;
    cam.translate(glm::vec3(0.0,2.0,1.0));
    cam.lookat(suzanne);
    render_scene main_scene(&cam);

    for(
        auto it = resources.begin<object>();
        it != resources.end<object>();
        ++it
    ){
        object* o = *it;
        if(o->get_model()) main_scene.add_object(o);
    }

    point_light l1(glm::vec3(1,0.5,0.5) * 3.0f);
    point_light l2(glm::vec3(0.5,0.5,1) * 3.0f);
    spotlight parrasvalo(glm::vec3(1,1,1)*3.0f);
    directional_light sun(glm::vec3(1,1,1)*5.0f);
    directional_light fake_sun;
    directional_shadow_map sun_shadow(
        w,
        glm::uvec2(1024),
        glm::vec3(0),
        glm::vec2(8.0f),
        glm::vec2(-5.0f, 5.0f),
        &fake_sun
    );
    sun_shadow.set_parent(suzanne);
    sun_shadow.set_bias(0.001, 0.03);

    parrasvalo.set_falloff_exponent(10);
    parrasvalo.set_position(glm::vec3(0.0f, 2.0f, 0.0f));

    main_scene.add_light(&l1);
    main_scene.add_light(&l2);
    main_scene.add_light(&parrasvalo);
    main_scene.add_light(&fake_sun);
    main_scene.add_shadow_map(&sun_shadow);

    glm::uvec2 render_resolution = w.get_size();

    deferred_pipeline dp(w, render_resolution, shaders, &main_scene);
    visualizer_pipeline vp(w, render_resolution, shaders, &main_scene);
    forward_pipeline fp(w, render_resolution, shaders, &main_scene);

    dp.sky.set_sun(&sun);
    dp.sky.set_intensity(5);
    fp.sky.set_sun(&sun);
    fp.sky.set_scaling(4/6.3781e6);
    fp.sky.set_radius(6.3781e6/4);
    fp.sky.set_samples(12,3);
    fp.sky.set_intensity(1);
    fp.sky.set_parent(earth);

    pipeline* pipelines[] = {&dp, &vp, &fp};

    bool running = true;
    unsigned pipeline_index = 2;
    bool paused = false;
    bool measure_times = false;
    float time = 0;
    float pitch = 0, yaw = 0;
    float speed = 2;
    float sensitivity = 0.2;
    float fov = 90;
    SDL_Event e;

    while(running)
    {
        while(w.poll(e))
        {
            switch(e.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                if(e.key.keysym.sym == SDLK_ESCAPE) running = false;
                if(e.key.keysym.sym == SDLK_PLUS) speed *= 1.1;
                if(e.key.keysym.sym == SDLK_MINUS) speed /= 1.1;
                if(e.key.keysym.sym == SDLK_1) pipeline_index = 0;
                if(e.key.keysym.sym == SDLK_2) pipeline_index = 1;
                if(e.key.keysym.sym == SDLK_3) pipeline_index = 2;
                if(e.key.keysym.sym == SDLK_RETURN) paused = !paused;
                if(e.key.keysym.sym == SDLK_t) measure_times = true;
                break;
            case SDL_MOUSEMOTION:
                pitch = std::clamp(pitch-e.motion.yrel*sensitivity, -90.0f, 90.0f);
                yaw -= e.motion.xrel*sensitivity;
                break;
            default:
                break;
            };
        }
        float delta = w.get_delta();
        const uint8_t* state = SDL_GetKeyboardState(NULL);
        if(state[SDL_SCANCODE_W]) cam.translate_local(glm::vec3(0,0,-1)*delta*speed);
        if(state[SDL_SCANCODE_S]) cam.translate_local(glm::vec3(0,0,1)*delta*speed);
        if(state[SDL_SCANCODE_A]) cam.translate_local(glm::vec3(-1,0,0)*delta*speed);
        if(state[SDL_SCANCODE_D]) cam.translate_local(glm::vec3(1,0,0)*delta*speed);
        if(state[SDL_SCANCODE_LSHIFT]) cam.translate_local(glm::vec3(0,-1,0)*delta*speed);
        if(state[SDL_SCANCODE_SPACE]) cam.translate_local(glm::vec3(0,1,0)*delta*speed);
        if(state[SDL_SCANCODE_Z]) fov -= delta*30;
        if(state[SDL_SCANCODE_X]) fov += delta*30;
        cam.perspective(fov, w.get_aspect(), 0.1, 20);
        cam.set_orientation(pitch, yaw);

        if(!paused)
        {
            time += w.get_delta();
            suzanne->rotate_local(w.get_delta()*60, glm::vec3(0,1,0));
            l1.set_position(glm::vec3(sin(time*2),2+sin(time*5),cos(time*2)));
            l2.set_position(glm::vec3(sin(time*2+M_PI),2-sin(time*5),cos(time*2+M_PI)));
            parrasvalo.set_orientation(time*50, glm::vec3(1,0,0));
            parrasvalo.set_cutoff_angle(sin(time)*45+45);
            earth->rotate(w.get_delta()*60, glm::vec3(0,1,0));
            sun.set_direction(glm::vec3(sin(time/2), cos(time/2), 0));
        }
        if(pipeline_index == 0)
        {
            fake_sun.set_color(
                dp.sky.get_attenuated_sun_color(cam.get_global_position())
            );
        }
        else
        {
            fake_sun.set_color(sun.get_color());
        }
        fake_sun.set_direction(sun.get_direction());

        if(!measure_times)
        {
            pipelines[pipeline_index]->execute();
        }
        else
        {
            std::vector<double> times;
            pipelines[pipeline_index]->execute(times);
            measure_times = false;

            std::cout << "Pipeline stage times: " << std::endl;
            double total = 0;
            for(unsigned i = 0; i < times.size(); ++i)
            {
                total += times[i];
                std::cout << "\tStage " << i + 1
                          << ": "
                          << times[i] << "ms"
                          << std::endl;
            }
            std::cout << "Total: " << total << std::endl;
        }

        w.present();
    }

    return 0;
}
