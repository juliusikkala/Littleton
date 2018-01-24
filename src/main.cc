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
    window w(768, 768, "dflowers", false, false);
    resource_store resources;
    resources.add_dfo("data/test.dfo");
    texture* cat_eyes_white = resources.add(
        "cat_eyes_white",
        texture::create("data/images/cat_eyes_white.png")
    );
    shader* effect_shader = resources.add("cat",
        shader::create_from_file(
            "data/shaders/fullscreen.vert",
            "data/shaders/test.frag",
            {
                {"TEXTURE_COLOR", "vec4(1.0,0.0,0.0,1.0)"}
            }
        )
    );

    method::clear clear_sky(glm::vec4(0.5, 0.5, 1.0, 0.0));
    method::fullscreen_effect effect(effect_shader);
    cat_eyes_white->load();

    pipeline p({&effect});
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
        cat_eyes_white->bind();
        effect_shader->set<float>("time", SDL_GetTicks()/1000.0f);
        effect_shader->set<int>("tex", 0);
        p.execute();
        w.present();
    }
    return 0;
}
