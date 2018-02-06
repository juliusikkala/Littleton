#include "doublebuffer.hh"

doublebuffer::doublebuffer(
    context& ctx,
    glm::uvec2 size,
    GLenum external_format,
    GLint internal_format,
    GLenum type
): glresource(ctx), cur_index(0),
   buffers{
    texture(ctx, size.x, size.y, external_format, internal_format, type),
    texture(ctx, size.x, size.y, external_format, internal_format, type)
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

doublebuffer::target& doublebuffer::input()
{
    return targets[cur_index];
}

const doublebuffer::target& doublebuffer::input() const
{
    return targets[cur_index];
}

texture& doublebuffer::output()
{
    return buffers[1-cur_index];
}

const texture& doublebuffer::output() const
{
    return buffers[1-cur_index];
}

void doublebuffer::swap()
{
    cur_index = 1-cur_index;
}
