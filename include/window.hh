#ifndef LT_WINDOW_HH
#define LT_WINDOW_HH
#include "render_target.hh"
#include "context.hh"
#include "math.hh"
#include <SDL.h>

namespace lt
{

class window: public context, public render_target
{
public:
    struct params
    {
        std::string title = "Littleton";
        glm::uvec2 size = glm::uvec2(640, 480);
        bool fullscreen = false;
        bool vsync = true;
        bool srgb = true;
        unsigned framerate_limit = 0;
        unsigned samples = 0;
    };

    window(const params& p);
    window(const window& other) = delete;
    window(window&& other) = delete;
    ~window();

    int poll(SDL_Event& event);
    void present();

    void set_framerate_limit(unsigned framerate_limit);
    unsigned get_framerate_limit() const;

    void grab_mouse(bool enabled = true);

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

} // namespace lt
#endif
