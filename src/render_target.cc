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
#include "render_target.hh"
#include <stdexcept>

namespace lt
{

GLint render_target::current_read_fbo = -1;
GLint render_target::current_write_fbo = -1;

render_target::render_target(context& ctx, GLenum target, glm::uvec2 size)
: glresource(ctx), fbo(0), target(target), size(size) {}

render_target::~render_target() {}

void render_target::bind(GLenum target)
{
    switch(target)
    {
    case GL_FRAMEBUFFER:
        if(fbo != (GLuint)current_read_fbo || fbo != (GLuint)current_write_fbo)
        {
            glBindFramebuffer(target, fbo);
            current_read_fbo = fbo;
            current_write_fbo = fbo;
        }
        break;
    case GL_READ_FRAMEBUFFER:
        if(fbo != (GLuint)current_read_fbo)
        {
            glBindFramebuffer(target, fbo);
            current_read_fbo = fbo;
        }
        break;
    case GL_DRAW_FRAMEBUFFER:
        if(fbo != (GLuint)current_write_fbo)
        {
            glBindFramebuffer(target, fbo);
            current_write_fbo = fbo;
        }
        break;
    default:
        throw std::runtime_error("Unknown render_target bind target");
    }
    glViewport(0, 0, size.x, size.y);
}

void render_target::unbind()
{
    if(fbo == (GLuint)current_read_fbo)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        current_read_fbo = -1;
    }

    if(fbo == (GLuint)current_write_fbo)
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        current_write_fbo = -1;
    }
}

bool render_target::is_bound(GLenum target) const
{
    switch(target)
    {
    case GL_FRAMEBUFFER:
        return fbo == (GLuint)current_read_fbo &&
               fbo == (GLuint)current_write_fbo;
    case GL_READ_FRAMEBUFFER:
        return fbo == (GLuint)current_read_fbo;
    case GL_DRAW_FRAMEBUFFER:
        return fbo == (GLuint)current_write_fbo;
    default:
        throw std::runtime_error("Unknown render_target unbind target");
    }
    return false;
}

GLenum render_target::get_target() const
{
    return target;
}

glm::uvec2 render_target::get_size() const
{
    return size;
}

float render_target::get_aspect() const
{
    return size.x/(float)size.y;
}

GLuint render_target::get_fbo() const
{
    return fbo;
}

GLint render_target::get_current_read_fbo()
{
    return current_read_fbo;
}

GLint render_target::get_current_write_fbo()
{
    return current_write_fbo;
}

void render_target::reinstate_current_fbo()
{
    if(current_read_fbo != -1)
        glBindFramebuffer(GL_READ_FRAMEBUFFER, current_read_fbo);

    if(current_write_fbo != -1)
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, current_write_fbo);
}

} // namespace lt
