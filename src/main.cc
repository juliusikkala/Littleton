#include "window.hh"
#include "resources.hh"
#include "object.hh"
#include <iostream>

int main()
{ 
    window w(640, 480, "dflowers", false);
    resource_store resources;
    resources.add_dfo("data/test.dfo");
    object_ptr cube = resources.get<object>("cube");
    model_ptr cube_model = cube->get_model();
    vertex_buffer_ptr cube_vb = (*cube_model)[0].vertex;

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
        w.swap();
    }
    return 0;
}
