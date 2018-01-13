#include "window.hh"
#include "resources.hh"

int main()
{ 
    window w(640, 480, "dflowers", false);
    resource_store resources;

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
