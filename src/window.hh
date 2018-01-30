#ifndef WINDOW_HH
#define WINDOW_HH
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

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

    void grab_mouse(bool enabled = true);

    int get_delta_ms() const;
    float get_delta() const;

    glm::uvec2 get_size() const;
    float get_aspect() const;

private:
    static bool initialized;

    unsigned w, h;
    unsigned framerate_limit;
    int last_frame;
    int delta;

    SDL_Window* win;
    SDL_GLContext ctx;
};
#endif
