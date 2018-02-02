#include "window.hh"
#include "resources.hh"
#include "texture.hh"
#include "object.hh"
#include "pipeline.hh"
#include "framebuffer.hh"
#include "method/clear.hh"
#include "method/fullscreen_effect.hh"
#include "method/forward_pass.hh"
#include "method/geometry_pass.hh"
#include "method/blit_framebuffer.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include <iostream>
#include <algorithm>
#include <glm/gtc/random.hpp>

int main()
{ 
    window w(1280, 720, "dflowers", true, true);
    w.set_framerate_limit(120);
    std::cout << w.get_vendor_name() << std::endl
              << w.get_renderer() << std::endl;

    w.grab_mouse();
    resource_store resources(w);
    resources.add_dfo("data/test_scene.dfo", "data");

    shader* effect_shader = resources.add("cat",
        shader::create_from_file(
            w,
            "data/shaders/fullscreen.vert",
            "data/shaders/test.frag"
        )
    );

    shader* texture_shader = resources.add("texture",
        shader::create_from_file(
            w,
            "data/shaders/fullscreen.vert",
            "data/shaders/texture.frag"
        )
    );

    shader_cache* geometry_shader = resources.add("drender",
        shader_cache::create_from_file(
            w,
            "data/shaders/generic.vert",
            "data/shaders/geometry.frag"
        )
    );

    shader_cache* forward_shader = resources.add("frender",
        shader_cache::create_from_file(
            w,
            "data/shaders/generic.vert",
            "data/shaders/forward.frag"
        )
    );
    object* suzanne = resources.get<object>("Suzanne");
    object* sphere = resources.get<object>("Sphere");
    object* cube = resources.get<object>("Cube");

    camera cam;
    cam.perspective(90, w.get_aspect(), 0.1, 20);
    cam.translate(glm::vec3(0.0,2.0,1.0));
    cam.lookat(suzanne);
    render_scene deferred_scene(&cam);

    for(
        auto it = resources.begin<object>();
        it != resources.end<object>();
        ++it
    ){
        object* o = *it;
        if(o->get_model()) deferred_scene.add_object(o);
    }

    render_scene forward_scene(&cam);

    point_light l1(glm::vec3(1,0.5,0.5) * 3.0f);
    point_light l2(glm::vec3(0.5,0.5,1) * 3.0f);
    spotlight parrasvalo(glm::vec3(1,1,1)*3.0f);
    parrasvalo.set_falloff_exponent(10);
    parrasvalo.set_position(glm::vec3(0.0f, 2.0f, 0.0f));

    forward_scene.add_light(&l1);
    forward_scene.add_light(&l2);
    forward_scene.add_light(&parrasvalo);
    forward_scene.add_object(cube);
    deferred_scene.remove_object(cube);


    gbuffer buf(w, w.get_size());
    method::clear clear(buf, glm::vec4(0.0, 0.0, 0.0, 0.0));
    method::fullscreen_effect sky(buf, effect_shader);
    method::geometry_pass gp(buf, geometry_shader, &deferred_scene);
    method::blit_framebuffer buf_to_window(w, buf);
    method::forward_pass fp(w, forward_shader, &forward_scene);

    pipeline p({&clear, &sky, &gp, &buf_to_window, &fp});

    bool running = true;
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

        suzanne->rotate_local(w.get_delta()*60, glm::vec3(0,1,0));
        l1.set_position(glm::vec3(sin(time*2),2+sin(time*5),cos(time*2)));
        l2.set_position(glm::vec3(sin(time*2+M_PI),2-sin(time*5),cos(time*2+M_PI)));
        parrasvalo.set_orientation(time*50, glm::vec3(1,0,0));
        parrasvalo.set_cutoff_angle(sin(time)*45+45);
        sphere->lookat(&cam);

        p.execute();
        w.present();
        time += w.get_delta();
    }
    return 0;
}
