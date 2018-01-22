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
        unsigned framerate_limit = 0,
        unsigned samples = 0
    );
    window(const window& other) = delete;
    ~window();

    int poll(SDL_Event& event);
    void present();

    void set_framerate_limit(unsigned framerate_limit);
    unsigned get_framerate_limit() const;

    int get_delta_ms() const;
    float get_delta() const;

private:
    static bool initialized;

    unsigned framerate_limit;
    int last_frame;
    int delta;

    SDL_Window* win;
    SDL_GLContext ctx;
};
#endif
