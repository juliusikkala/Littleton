#ifndef WINDOW_HH
#define WINDOW_HH
#include <SDL2/SDL.h>

class window
{
public:
    window(
        unsigned w = 640,
        unsigned h = 480,
        const char* title = "dflowers",
        bool fullscreen = false,
        bool vsync = true,
        unsigned samples = 0
    );
    window(const window& other) = delete;
    ~window();

    int poll(SDL_Event& event);
    void swap();

private:
    static bool initialized;

    SDL_Window* win;
    SDL_GLContext ctx;
};
#endif
