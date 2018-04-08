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
#include "environment_map.hh"
#include "method/clear.hh"
#include "method/clear_gbuffer.hh"
#include "method/forward_pass.hh"
#include "method/geometry_pass.hh"
#include "method/lighting_pass.hh"
#include "method/blit_framebuffer.hh"
#include "method/visualize_gbuffer.hh"
#include "method/tonemap.hh"
#include "method/sky.hh"
#include "method/skybox.hh"
#include "method/shadow_pcf.hh"
#include "method/shadow_msm.hh"
#include "method/draw_texture.hh"
#include "method/ssao.hh"
#include "method/ssrt.hh"
#include "method/sao.hh"
#include "method/bloom.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include "doublebuffer.hh"
#include <iostream>
#include <algorithm>
#include <glm/gtc/random.hpp>

class game_pipelines
{
public:
    game_pipelines(
        window& w,
        glm::uvec2 resolution,
        resource_pool& pool,
        render_scene* main_scene
    ):  normal(w, resolution, GL_RG16_SNORM, GL_UNSIGNED_BYTE),
        color_emission(w, resolution, GL_RGBA8, GL_UNSIGNED_BYTE),
        material(w, resolution, GL_RGBA8, GL_UNSIGNED_BYTE),
        lighting(w, resolution, GL_RGB16F, GL_FLOAT),
        linear_depth(w, resolution, GL_RG16F, GL_FLOAT),
        depth_stencil(w, resolution, GL_DEPTH24_STENCIL8, GL_UNSIGNED_INT_24_8),

        buf(
            w,
            resolution,
            &normal,
            &color_emission,
            &material,
            &lighting,
            &linear_depth,
            &depth_stencil
        ),
        postprocess(w, resolution, GL_RGB16F),

        clear_buf(buf),
        skybox(buf, pool, main_scene),

        pcf(pool, main_scene),
        msm(pool, main_scene),

        gp(buf, pool, main_scene),
        lp(buf, buf, pool, main_scene, false),
        fp(buf, pool, main_scene, false),
        visualizer(buf, buf, pool, main_scene),
        dt(w, pool),

        sky(
            buf,
            pool,
            main_scene,
            buf.get_depth_stencil()
        ),
        bloom(
            postprocess.input(0),
            pool,
            &lighting,
            6.0f, 10, 0.1f
        ),
        tm(
            postprocess.input(1),
            pool,
            &postprocess.output(1)
        ),
        sao(buf, buf, pool, main_scene, 1.0f, 8, 0.01f, 0.5f),
        ssrt(buf, buf, pool, main_scene),

        postprocess_to_window(
            w,
            postprocess.input(1),
            method::blit_framebuffer::COLOR_ONLY
        ),
        buf_to_window(
             w,
             buf,
             method::blit_framebuffer::COLOR_ONLY
        ),
        forward_pipeline({
            &pcf,
            &msm,
            &clear_buf,
            &skybox,
            &fp,
            &sao,
            &sky,
            &bloom,
            &tm,
            &postprocess_to_window
        }),
        visualizer_pipeline({
            &clear_buf,
            &skybox,
            &gp,
            &visualizer,
            &buf_to_window
        }),
        deferred_pipeline({
            &pcf,
            &msm,
            &clear_buf,
            &gp,
            &skybox,
            &lp,
            &sao,
            &ssrt,
            &sky,
            &bloom,
            &tm,
            &postprocess_to_window
        }),
        texture_pipeline({&pcf, &msm, &dt})
    {
        fp.render_transparent(false);
        dt.set_texture(&linear_depth);
    }

    method::shadow_msm& get_msm() { return msm; }
    method::shadow_pcf& get_pcf() { return pcf; }
    method::lighting_pass& get_lighting() { return lp; }

    method::sky& get_sky() { return sky; }
    method::skybox& get_skybox() { return skybox; }
    void set_texture(texture* tex) { dt.set_texture(tex); }

    pipeline* get_forward_pipeline() { return &forward_pipeline; }
    pipeline* get_visualizer_pipeline() { return &visualizer_pipeline; }
    pipeline* get_deferred_pipeline() { return &deferred_pipeline; }
    pipeline* get_texture_pipeline() { return &texture_pipeline; }

private:
    texture normal;
    texture color_emission;
    texture material;
    texture lighting;
    texture linear_depth;
    texture depth_stencil;

    gbuffer buf;
    doublebuffer postprocess;

    method::clear_gbuffer clear_buf;
    method::skybox skybox;

    method::shadow_pcf pcf;
    method::shadow_msm msm;

    method::geometry_pass gp;
    method::lighting_pass lp;
    method::forward_pass fp;
    method::visualize_gbuffer visualizer;
    method::draw_texture dt;

    method::sky sky;
    method::bloom bloom;
    method::tonemap tm;
    method::ssrt ssrt;
    method::sao sao;

    method::blit_framebuffer postprocess_to_window;
    method::blit_framebuffer buf_to_window;

    pipeline forward_pipeline;
    pipeline visualizer_pipeline;
    pipeline deferred_pipeline;
    pipeline texture_pipeline;
};

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

        environment.reset(environment_map::create(
            win, "data/environment/venice.hdr"
        ));

        glm::uvec2 render_resolution = win.get_size();

        pipelines.reset(new game_pipelines(
            win, render_resolution, resources, &main_scene
        ));

        sun_shadow_pcf.reset(
            new directional_shadow_map_pcf(
                &pipelines->get_pcf(),
                win,
                glm::uvec2(1024),
                4,
                4,
                glm::vec3(0),
                glm::vec2(8.0f),
                glm::vec2(-5.0f, 5.0f),
                &sun
            )
        );

        sun_shadow_msm.reset(
            new directional_shadow_map_msm(
                &pipelines->get_msm(),
                win,
                glm::uvec2(1024),
                0,
                4,
                glm::vec3(0),
                glm::vec2(8.0f),
                glm::vec2(-5.0f, 5.0f),
                &sun
            )
        );

        fly_shadow_pcf.reset(
            new omni_shadow_map_pcf(
                &pipelines->get_pcf(),
                win,
                glm::uvec2(256),
                16,
                0.1f,
                glm::vec2(0.01f, 5.0f),
                &l1
            )
        );

        fly_shadow_msm.reset(
            new omni_shadow_map_msm(
                &pipelines->get_msm(),
                win,
                glm::uvec2(256),
                0,
                1.0f,
                glm::vec2(0.01f, 5.0f),
                &l2
            )
        );

        spot_shadow.reset(
            new perspective_shadow_map_msm(
                &pipelines->get_msm(),
                win,
                glm::uvec2(256),
                0,
                2.0f,
                30,
                glm::vec2(0.01f, 5.0f),
                &spot
            )
        );

        //pipelines->set_texture(&spot_shadow->get_moments());
        current_pipeline = pipelines->get_deferred_pipeline();
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

        //main_scene.add_light(&sun);
        //main_scene.add_light(&l1);
        //main_scene.add_light(&l2);
        main_scene.add_light(&spot);
        //main_scene.add_shadow(sun_shadow_msm.get());
        //main_scene.add_shadow(fly_shadow_pcf.get());
        //main_scene.add_shadow(fly_shadow_msm.get());
        main_scene.add_shadow(spot_shadow.get());
        main_scene.set_ambient(glm::vec3(0.03));
        main_scene.set_skybox(environment.get());

        //pipelines->get_lighting().set_visualize_light_volumes(true);

        method::sky& sky = pipelines->get_sky();
        sky.set_sun(&sun);
        sky.set_scaling(1/6.3781e6);
        sky.set_radius(6.3781e6);
        sky.set_samples(12,3);
        sky.set_intensity(1);
        sky.set_parent(earth);

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
                if(e.key.keysym.sym == SDLK_1)
                    current_pipeline = pipelines->get_deferred_pipeline();
                if(e.key.keysym.sym == SDLK_2)
                    current_pipeline = pipelines->get_visualizer_pipeline();
                if(e.key.keysym.sym == SDLK_3)
                    current_pipeline = pipelines->get_forward_pipeline();
                if(e.key.keysym.sym == SDLK_4)
                    current_pipeline = pipelines->get_texture_pipeline();
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
            l1.set_position(glm::vec3(sin(time*2),2,cos(time*2)));
            l2.set_position(glm::vec3(
                sin(time*2+M_PI),
                2-sin(time*5),
                cos(time*2+M_PI)
            ));
            spot.set_orientation(time*50, glm::vec3(1,0,0));
            //spot.set_cutoff_angle(sin(time)*45+45);
            //spot_shadow->set_fov(spot.get_cutoff_angle());
            spot.set_cutoff_angle(45);
            spot_shadow->set_fov(90);
            earth->rotate(delta*60, glm::vec3(0,1,0));
            sun.set_direction(glm::vec3(sin(time/2), cos(time/2), 0));
            pipelines->get_skybox().set_exposure(sin(time)+1);
        }

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

    std::unique_ptr<directional_shadow_map_pcf> sun_shadow_pcf;
    std::unique_ptr<directional_shadow_map_msm> sun_shadow_msm;
    std::unique_ptr<omni_shadow_map_pcf> fly_shadow_pcf;
    std::unique_ptr<omni_shadow_map_msm> fly_shadow_msm;
    std::unique_ptr<perspective_shadow_map_msm> spot_shadow;
    std::unique_ptr<environment_map> environment;
    std::unique_ptr<game_pipelines> pipelines;

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
