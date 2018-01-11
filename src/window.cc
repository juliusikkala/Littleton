#include "window.hh"
#include <stdexcept>
#include "glheaders.hh"
#include <SDL2/SDL_opengl.h>

bool window::initialized = false;

window::window(
    unsigned w,
    unsigned h,
    const char* title,
    bool fullscreen,
    bool vsync,
    unsigned samples
) {
    if(initialized)
    {
        throw std::runtime_error("There can be only one window.");
    }

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS))
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

    if(samples != 0)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, samples);
    }
    else
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, vsync);
    win = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        w, h,
        SDL_WINDOW_OPENGL | (fullscreen && SDL_WINDOW_FULLSCREEN)
    );

    if(!win)
    {
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }

    glewExperimental = GL_TRUE;
    glewInit();

    initialized = true;
}

int window::poll(SDL_Event& event)
{
    return SDL_PollEvent(&event);
}

void window::swap()
{
    SDL_GL_SwapWindow(win);
}

window::~window()
{
    SDL_Quit();
    initialized = false;    
}
