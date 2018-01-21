#include "window.hh"
#include "resources.hh"
#include "texture.hh"
#include "object.hh"
#include "pipeline.hh"
#include "method/clear.hh"
#include <iostream>

int main()
{ 
    window w(640, 480, "dflowers", false, false);
    resource_store resources;
    resources.add_dfo("data/test.dfo");
    method::clear clear_sky(glm::vec4(0.5, 0.5, 1.0, 0.0));
    pipeline p(
        clear_sky
    );

    for(auto it = resources.begin<texture>(); it != resources.end<texture>(); ++it)
    {
        std::cout<<it.name()<<std::endl;
    }

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
        p.execute();
        w.swap();
    }
    return 0;
}
