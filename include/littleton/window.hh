/*
    Copyright 2018 Julius Ikkala

    This file is part of Littleton.

    Littleton is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Littleton is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Littleton.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef LT_WINDOW_HH
#define LT_WINDOW_HH
#include "../api.hh"
#include "render_target.hh"
#include "context.hh"
#include "math.hh"
#include <SDL.h>

namespace lt
{

class LT_API window: public context, public render_target
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
