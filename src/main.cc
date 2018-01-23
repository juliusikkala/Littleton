#include "window.hh"
#include "resources.hh"
#include "texture.hh"
#include "object.hh"
#include "pipeline.hh"
#include "method/clear.hh"
#include "method/fullscreen_effect.hh"
#include "helpers.hh"
#include <iostream>

int main()
{ 
    window w(640, 480, "dflowers", false, false);
    resource_store resources;
    resources.add(
        "cat_eyes_white",
        texture::create("data/images/cat_eyes_white.png")
    );
    resources.add_dfo("data/test.dfo");
    method::clear clear_sky(glm::vec4(0.5, 0.5, 1.0, 0.0));
    method::fullscreen_effect effect(
        read_text_file("data/shaders/test.frag")
    );
    texture* cat_eyes_white = resources.get<texture>("cat_eyes_white");
    cat_eyes_white->load();

    pipeline p({
        &effect
    });
    w.set_framerate_limit(60);

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
        effect.get_shader()->set<float>("time", SDL_GetTicks()/1000.0f);
        p.execute();
        w.present();
    }
    return 0;
}
