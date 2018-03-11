#include "window.hh"
#include "resource_pool.hh"
#include "loaders.hh"
#include "texture.hh"
#include "object.hh"
#include "camera.hh"
#include "scene.hh"
#include "scene_graph.hh"
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
#include "method/shadow_pcf.hh"
#include "method/shadow_msm.hh"
#include "method/draw_texture.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include "doublebuffer.hh"
#include <iostream>
#include <algorithm>
#include <glm/gtc/random.hpp>

struct deferred_data
{
    deferred_data(
        window& w,
        glm::uvec2 resolution,
        resource_pool& pool,
        render_scene* main_scene
    ):  screen(w, resolution, GL_RGB16F, GL_FLOAT),
        buf(w, resolution),
        pcf(pool, main_scene),
        msm(pool, main_scene),
        clear_buf(buf),
        clear_screen(screen.input(0)),
        gp(buf, pool, main_scene),
        lp(screen.input(0), buf, pool, main_scene, {&pcf, &msm}),
        sky(screen.input(0), pool, main_scene, &buf.get_depth_stencil()),
        tm(screen.input(1), pool, &screen.output(1)),
        screen_to_window(
            w, screen.input(1),
            method::blit_framebuffer::COLOR_ONLY
        )
    {
        screen.set_depth_stencil(0, &buf.get_depth_stencil());
    }

    const std::vector<pipeline_method*> get_methods()
    {
        return {
            &pcf,
            &msm,
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

    method::shadow_pcf pcf;
    method::shadow_msm msm;
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
        resource_pool& pool,
        render_scene* main_scene
    ): screen(w, resolution, GL_RGB16F, GL_FLOAT),
       buf(w, resolution),
       clear_buf(buf),
       clear_screen(screen.input(0)),
       gp(buf, pool, main_scene),
       visualizer(screen.input(0), buf, pool, main_scene),
       screen_to_window(
            w,
            screen.input(0),
            method::blit_framebuffer::COLOR_ONLY
        )
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
        resource_pool& pool,
        render_scene* main_scene
    ):  screen(w, resolution, {
            {GL_COLOR_ATTACHMENT0, {GL_RGB16F, true}},
            {GL_DEPTH_ATTACHMENT, {GL_DEPTH24_STENCIL8, true}}
        }),
        postprocess(w, resolution, GL_RGB16F, GL_FLOAT),
        pcf(pool, main_scene),
        msm(pool, main_scene),
        clear_screen(screen),
        fp(screen, pool, main_scene, {&pcf, &msm}),
        sky(
            screen,
            pool,
            main_scene,
            screen.get_texture_target(GL_DEPTH_ATTACHMENT)
        ),
        tm(
            postprocess.input(0),
            pool,
            screen.get_texture_target(GL_COLOR_ATTACHMENT0)
        ),
        postprocess_to_window(
          w,
          postprocess.input(0),
          method::blit_framebuffer::COLOR_ONLY
        )
    {}


    const std::vector<pipeline_method*> get_methods()
    {
        return {
            &pcf,
            &msm,
            &clear_screen,
            &fp,
            &sky,
            &tm,
            &postprocess_to_window
        };
    }

    framebuffer screen;
    doublebuffer postprocess;

    method::shadow_pcf pcf;
    method::shadow_msm msm;
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
        resource_pool& pool,
        render_scene* main_scene
    ): T(w, resolution, pool, main_scene), pipeline(T::get_methods()) {}
};

using deferred_pipeline = custom_pipeline<deferred_data>;
using visualizer_pipeline = custom_pipeline<visualizer_data>;
using forward_pipeline = custom_pipeline<forward_data>;

class game
{
public:
    game()
        : win({ "dflowers", {1280, 720}, true, true, false }),
        resources(win, { "data/shaders/" })
    {
        win.set_framerate_limit(200);
        win.grab_mouse();
        std::cout << "GPU Vendor: " << win.get_vendor_name() << std::endl
            << "Renderer:   " << win.get_renderer() << std::endl;
    }

    ~game()
    {
    }

    void load()
    {
        load_dfo(resources, graph, "data/test_scene.dfo", "data");
        load_dfo(resources, graph, "data/earth.dfo", "data");

        sun_shadow_pcf.reset(
            new directional_shadow_map_pcf(
                win,
                glm::uvec2(1024),
                4,
                4,
                glm::vec3(0),
                glm::vec2(8.0f),
                glm::vec2(-5.0f, 5.0f),
                &fake_sun
            )
        );

        sun_shadow_msm.reset(
            new directional_shadow_map_msm(
                win,
                glm::uvec2(1024),
                4,
                4,
                glm::vec3(0),
                glm::vec2(8.0f),
                glm::vec2(-5.0f, 5.0f),
                &fake_sun
            )
        );

        glm::uvec2 render_resolution = win.get_size();

        dp.reset(new deferred_pipeline(
            win, render_resolution, resources, &main_scene
        ));
        vp.reset(new visualizer_pipeline(
            win, render_resolution, resources, &main_scene
        ));
        fp.reset(new forward_pipeline(
            win, render_resolution, resources, &main_scene
        ));

        current_pipeline = fp.get();
    }

    void setup_scene()
    {
        object* suzanne = graph.get_object("Suzanne");
        object* earth = graph.get_object("Earth");
        earth->set_position(glm::vec3(0, 2, -6));
        earth->scale(2);

        cam.translate(glm::vec3(0.0,2.0,1.0));
        cam.lookat(suzanne);
        main_scene.set_camera(&cam);

        sun_shadow_pcf->set_parent(suzanne);
        sun_shadow_msm->set_parent(suzanne);

        graph.add_to_scene(&main_scene);

        l1.set_color(glm::vec3(1,0.5,0.5) * 3.0f);
        l2.set_color(glm::vec3(0.5,0.5,1) * 3.0f);

        spot.set_color(glm::vec3(1,1,1) * 3.0f);
        spot.set_falloff_exponent(10);
        spot.set_position(glm::vec3(0.0f, 2.0f, 0.0f));

        sun.set_color(glm::vec3(1,1,1) * 5.0f);

        main_scene.add_light(&l1);
        main_scene.add_light(&l2);
        main_scene.add_light(&spot);
        main_scene.add_light(&fake_sun);

        dp->msm.add(sun_shadow_msm.get());
        //dp->pcf.add(sun_shadow_pcf.get());
        fp->msm.add(sun_shadow_msm.get());
        //fp->pcf.add(sun_shadow_pcf.get());

        dp->sky.set_sun(&sun);
        dp->sky.set_intensity(5);
        dp->sky.set_samples(8,2);
        fp->sky.set_sun(&sun);
        fp->sky.set_scaling(1/6.3781e6);
        fp->sky.set_radius(6.3781e6);
        fp->sky.set_samples(12,3);
        fp->sky.set_intensity(1);
        fp->sky.set_parent(earth);

        paused = false;
        measure_times = false;
        time = 0;
        pitch = 0;
        yaw = 0;
        speed = 2;
        sensitivity = 0.2;
        fov = 90;
    }

    // Return false if the program should quit.
    bool update()
    {
        SDL_Event e;

        while(win.poll(e))
        {
            switch(e.type)
            {
            case SDL_QUIT:
                return false;
            case SDL_KEYDOWN:
                if(e.key.keysym.sym == SDLK_ESCAPE) return false;
                if(e.key.keysym.sym == SDLK_PLUS) speed *= 1.1;
                if(e.key.keysym.sym == SDLK_MINUS) speed /= 1.1;
                if(e.key.keysym.sym == SDLK_1) current_pipeline = dp.get();
                if(e.key.keysym.sym == SDLK_2) current_pipeline = vp.get();
                if(e.key.keysym.sym == SDLK_3) current_pipeline = fp.get();
                if(e.key.keysym.sym == SDLK_RETURN) paused = !paused;
                if(e.key.keysym.sym == SDLK_t) measure_times = true;
                if(e.key.keysym.sym == SDLK_r)
                {
                    resources.delete_binaries();
                    resources.unload_all();
                }
                break;
            case SDL_MOUSEMOTION:
                pitch = std::clamp(
                    pitch-e.motion.yrel*sensitivity, -90.0f, 90.0f
                );
                yaw -= e.motion.xrel*sensitivity;
                break;
            default:
                break;
            };
        }
        float delta = win.get_delta();
        const uint8_t* state = SDL_GetKeyboardState(NULL);
        if(state[SDL_SCANCODE_W])
            cam.translate_local(glm::vec3(0,0,-1)*delta*speed);
        if(state[SDL_SCANCODE_S])
            cam.translate_local(glm::vec3(0,0,1)*delta*speed);
        if(state[SDL_SCANCODE_A])
            cam.translate_local(glm::vec3(-1,0,0)*delta*speed);
        if(state[SDL_SCANCODE_D])
            cam.translate_local(glm::vec3(1,0,0)*delta*speed);
        if(state[SDL_SCANCODE_LSHIFT])
            cam.translate_local(glm::vec3(0,-1,0)*delta*speed);
        if(state[SDL_SCANCODE_SPACE])
            cam.translate_local(glm::vec3(0,1,0)*delta*speed);
        if(state[SDL_SCANCODE_Z])
            fov -= delta*30;
        if(state[SDL_SCANCODE_X])
            fov += delta*30;
        cam.perspective(fov, win.get_aspect(), 0.1, 20);
        cam.set_orientation(pitch, yaw);

        object* suzanne = graph.get_object("Suzanne");
        object* earth = graph.get_object("Earth");

        if(!paused)
        {
            time += delta;
            suzanne->rotate_local(delta*60, glm::vec3(0,1,0));
            l1.set_position(glm::vec3(sin(time*2),2+sin(time*5),cos(time*2)));
            l2.set_position(glm::vec3(
                sin(time*2+M_PI),
                2-sin(time*5),
                cos(time*2+M_PI)
            ));
            spot.set_orientation(time*50, glm::vec3(1,0,0));
            spot.set_cutoff_angle(sin(time)*45+45);
            earth->rotate(delta*60, glm::vec3(0,1,0));
            sun.set_direction(glm::vec3(sin(time/2), cos(time/2), 0));
        }

        if(current_pipeline == dp.get())
        {
            fake_sun.set_color(dp->sky.get_attenuated_sun_color(
                cam.get_global_position()
            ));
        }
        else
        {
            fake_sun.set_color(sun.get_color());
        }
        fake_sun.set_direction(sun.get_direction());

        return true;
    }

    void render()
    {
        if(current_pipeline)
        {
            if(!measure_times)
            {
                current_pipeline->execute();
            }
            else
            {
                std::vector<double> times;
                current_pipeline->execute(times);
                measure_times = false;

                std::cout << "Pipeline stage times: " << std::endl;
                double total = 0;
                for(unsigned i = 0; i < times.size(); ++i)
                {
                    total += times[i];
                    std::cout << "\tStage " << i + 1
                              << " (" << current_pipeline->get_name(i) << "): "
                              << times[i] << "ms"
                              << std::endl;
                }
                std::cout << "Total: " << total << std::endl;
            }

            win.present();
        }
    }

private:
    window win;
    resource_pool resources;
    scene_graph graph;
    camera cam;
    render_scene main_scene;

    point_light l1;
    point_light l2;
    spotlight spot;
    directional_light sun;
    // This is a copy of 'sun' approximately colored by the atmosphere
    directional_light fake_sun;

    std::unique_ptr<directional_shadow_map_pcf> sun_shadow_pcf;
    std::unique_ptr<directional_shadow_map_msm> sun_shadow_msm;
    std::unique_ptr<deferred_pipeline> dp;
    std::unique_ptr<visualizer_pipeline> vp;
    std::unique_ptr<forward_pipeline> fp;

    pipeline* current_pipeline;

    bool paused;
    bool measure_times;
    float time;
    float pitch, yaw;
    float speed;
    float sensitivity;
    float fov;
};

int main(int argc, char** argv)
{ 
    try
    {
        game g;

        g.load();
        g.setup_scene();

        while(g.update())
        {
            g.render();
        }
    }
    catch(const std::runtime_error& err)
    {
        std::string what = err.what();

        std::cout << "Runtime error: " << std::endl
                  << what << std::endl;

        write_binary_file(
            "error.txt",
            (const uint8_t*)what.c_str(),
            what.length()
        );

        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Runtime error",
            "Error report written to error.txt",
            nullptr
        );
        return 1;
    }
    return 0;
}
