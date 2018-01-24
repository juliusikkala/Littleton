#include "window.hh"
#include "resources.hh"
#include "texture.hh"
#include "object.hh"
#include "pipeline.hh"
#include "method/clear.hh"
#include "method/fullscreen_effect.hh"
#include "method/forward_render.hh"
#include "helpers.hh"
#include <iostream>

int main()
{ 
    window w(1280, 720, "dflowers", false, true);
    resource_store resources;
    resources.add_dfo("data/test_scene.dfo");

    texture* cat_eyes_white = resources.add(
        "cat_eyes_white",
        texture::create("data/images/cat_eyes_white.png")
    );
    shader* effect_shader = resources.add("cat",
        shader::create_from_file(
            "data/shaders/fullscreen.vert",
            "data/shaders/test.frag", {
                {"TEXTURE_COLOR", "vec4(1.0,0.0,0.0,1.0)"}
            }
        )
    );
    shader_cache* render_shader = resources.add("render",
        new shader_cache(
            read_text_file("data/shaders/generic.vert"),
            read_text_file("data/shaders/forward_render.frag")
        )
    );

    camera cam;
    cam.perspective(60, w.get_aspect(), 0.1, 10);
    cam.translate(glm::vec3(2.0,2.0,2.0));
    scene main_scene(&cam);

    for(auto it = resources.begin<object>(); it != resources.end<object>(); ++it)
    {
        object* o = *it;
        if(o->get_model()) main_scene.add(o);
    }

    method::clear clear(glm::vec4(0.5, 0.5, 1.0, 0.0));
    method::fullscreen_effect effect(effect_shader);
    method::forward_render render(render_shader, &main_scene);

    pipeline p({&clear, &effect, &render});

    bool running = true;
    while(running)
    {
        SDL_Event e;

        while(w.poll(e))
        {
            switch(e.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            default:
                break;
            };
        }
        cam.set_orientation(-25, 45 + sin(SDL_GetTicks()/400.0f)*15);
        p.execute();
        w.present();
    }
    return 0;
}
