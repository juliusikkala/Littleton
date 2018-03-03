#include "doublebuffer.hh"
#include <stdexcept>

doublebuffer::doublebuffer(
    context& ctx,
    glm::uvec2 size,
    GLenum external_format,
    GLint internal_format,
    GLenum type
): glresource(ctx), cur_index(0),
   buffers{
    texture(ctx, size, external_format, internal_format, type),
    texture(ctx, size, external_format, internal_format, type)
   },
   targets{target(ctx, buffers[0]), target(ctx, buffers[1])}
{
}

doublebuffer::doublebuffer(doublebuffer&& other)
: glresource(other.get_context()), cur_index(other.cur_index),
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
: render_target(ctx, tex.get_size())
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

void doublebuffer::swap()
{
    cur_index = 1-cur_index;
}
