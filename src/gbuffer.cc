#include "gbuffer.hh"
#include <stdexcept>

gbuffer::gbuffer(context& ctx, glm::uvec2 size)
: render_target(ctx, size),
  depth_stencil(
    ctx,
    size,
    GL_DEPTH24_STENCIL8,
    GL_UNSIGNED_INT_24_8
  ),
  color_emission(ctx, size, GL_RGBA8, GL_UNSIGNED_BYTE),
  normal(ctx, size, GL_RG16_SNORM, GL_UNSIGNED_BYTE),
  material(ctx, size, GL_RGBA8, GL_UNSIGNED_BYTE)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        depth_stencil.get_target(),
        depth_stencil.get_texture(),
        0
    );

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        color_emission.get_target(),
        color_emission.get_texture(),
        0
    );

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT1,
        normal.get_target(),
        normal.get_texture(),
        0
    );

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT2,
        material.get_target(),
        material.get_texture(),
        0
    );

    unsigned attachments[] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(sizeof(attachments)/sizeof(unsigned), attachments);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("GBuffer is incomplete!");

    reinstate_current_fbo();
}

gbuffer::gbuffer(gbuffer&& other)
: render_target(other), depth_stencil(std::move(other.depth_stencil)),
  color_emission(std::move(other.color_emission)),
  normal(std::move(other.normal)),
  material(std::move(other.material))
{
    other.fbo = 0;
}

gbuffer::~gbuffer()
{
    if(fbo != 0) glDeleteFramebuffers(1, &fbo);
}

texture& gbuffer::get_depth_stencil() { return depth_stencil; }
texture& gbuffer::get_color_emission() { return color_emission; }
texture& gbuffer::get_normal() { return normal; }
texture& gbuffer::get_material() { return material; }
