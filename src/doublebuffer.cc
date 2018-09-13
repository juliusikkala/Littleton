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
#include "doublebuffer.hh"
#include "helpers.hh"
#include <stdexcept>

namespace lt
{

doublebuffer::doublebuffer(
    context& ctx,
    glm::uvec2 size,
    GLint internal_format
):  glresource(ctx), cur_index(0),
    buffers{
        texture(
            ctx,
            size,
            internal_format,
            internal_format_compatible_type(internal_format)
        ),
        texture(
            ctx,
            size,
            internal_format,
            internal_format_compatible_type(internal_format)
        )
    },
    targets{target(ctx, buffers[0]), target(ctx, buffers[1])}
{
}

doublebuffer::doublebuffer(doublebuffer&& other)
:   glresource(other.get_context()), cur_index(other.cur_index),
    buffers{
        std::move(other.buffers[0]),
        std::move(other.buffers[1])
    },
    targets{
        std::move(other.targets[0]),
        std::move(other.targets[1])
    }
{
}

doublebuffer::~doublebuffer(){}

doublebuffer::target::target(context& ctx, texture& tex)
: render_target(ctx, tex.get_target(), tex.get_dimensions())
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        tex.get_target(),
        tex.get_texture(),
        0
    );

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Doublebuffer target is incomplete!");

    reinstate_current_fbo();
}

doublebuffer::target::target(target&& other)
: render_target(other)
{
    other.fbo = 0;
}

doublebuffer::target::~target()
{
    if(fbo != 0) glDeleteFramebuffers(1, &fbo);
}

void doublebuffer::target::set_depth_stencil(texture* depth_stencil)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        depth_stencil->get_target(),
        depth_stencil->get_texture(),
        0
    );
    reinstate_current_fbo();
}

doublebuffer::target& doublebuffer::input(unsigned index)
{
    return targets[index^cur_index];
}

const doublebuffer::target& doublebuffer::input(unsigned index) const
{
    return targets[index^cur_index];
}

texture& doublebuffer::output(unsigned index)
{
    return buffers[1-(index^cur_index)];
}

const texture& doublebuffer::output(unsigned index) const
{
    return buffers[1-(index^cur_index)];
}

void doublebuffer::set_depth_stencil(texture* depth_stencil)
{
    set_depth_stencil(0, depth_stencil);
}

void doublebuffer::set_depth_stencil(unsigned index, texture* depth_stencil)
{
    return targets[index^cur_index].set_depth_stencil(depth_stencil);
}

texture& doublebuffer::get_texture(unsigned actual_index)
{
    return buffers[actual_index];
}

void doublebuffer::swap()
{
    cur_index = 1-cur_index;
}

} // namespace lt
