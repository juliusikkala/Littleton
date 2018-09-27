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
#include "window.hh"
#include "glheaders.hh"
#include <stdexcept>
#include <thread>
#include <SDL_opengl.h>

namespace lt
{

bool window::initialized = false;

window::window(const params& p)
: render_target(*this, GL_TEXTURE_2D, glm::uvec3(p.size, 1)),
  framerate_limit(p.framerate_limit), last_delta(duration::zero())
{
    if(initialized)
    {
        throw std::runtime_error("There can be only one window.");
    }

    if(SDL_Init(SDL_INIT_EVERYTHING))
    {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE
    );
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GL_MAJOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GL_MINOR);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, (int)p.srgb);

    if(p.samples != 0)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, p.samples);
    }
    else
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    win = SDL_CreateWindow(
        p.title.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        p.size.x, p.size.y,
        SDL_WINDOW_OPENGL | (p.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)
    );

    if(!win)
    {
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }
    SDL_GetWindowSize(
        win,
        (int*)&(this->dimensions.x),
        (int*)&(this->dimensions.y)
    );

    ctx = SDL_GL_CreateContext(win);
    if(!ctx)
    {
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }
    SDL_GL_MakeCurrent(win, ctx);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if(err != GLEW_OK)
    {
        throw std::runtime_error((const char*)glewGetErrorString(err));
    }

    initialized = true;

    SDL_GL_SetSwapInterval(p.vsync);

    context::init();

    //Enable generic options
    if(p.srgb) glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

window::~window()
{
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    initialized = false;    
}

int window::poll(SDL_Event& event)
{
    return SDL_PollEvent(&event);
}

void window::present()
{
    SDL_GL_SwapWindow(win);

    if(last_delta == duration::zero()) frame_timer.lap();
    if(framerate_limit)
    {
        duration elapsed = frame_timer.elapsed();
        duration required = std::chrono::duration_cast<duration>(
            std::chrono::duration<double>(1.0/framerate_limit)
        );
        duration sleep_time = required - elapsed;

        if(elapsed < required) std::this_thread::sleep_for(sleep_time);
    }
    last_delta = frame_timer.lap();
}

void window::set_framerate_limit(unsigned framerate_limit)
{
    this->framerate_limit = framerate_limit;
}

unsigned window::get_framerate_limit() const
{
    return framerate_limit;
}

void window::grab_mouse(bool enabled)
{
    SDL_SetWindowGrab(win, (SDL_bool)enabled);
    SDL_SetRelativeMouseMode((SDL_bool)enabled);
}

double window::get_delta_sec() const
{
    return std::chrono::duration<double>(last_delta).count();
}

duration window::get_delta() const
{
    return last_delta;
}

} // namespace lt
