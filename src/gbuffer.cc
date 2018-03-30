#include "gbuffer.hh"
#include <stdexcept>

gbuffer::gbuffer(
    context& ctx,
    glm::uvec2 size,
    texture* lighting_target
): render_target(ctx, size),
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

    if(lighting_target)
    {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT3,
            lighting_target->get_target(),
            lighting_target->get_texture(),
            0
        );
        glReadBuffer(GL_COLOR_ATTACHMENT3);
    }

    draw_lighting();

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

void gbuffer::clear()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glStencilMask(0xFF);
    glClearColor(0, 0, 0, 1);
    glClearDepth(1);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    reinstate_current_fbo();
}

void gbuffer::draw_all()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    unsigned attachments[] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3
    };
    glDrawBuffers(sizeof(attachments)/sizeof(unsigned), attachments);
    reinstate_current_fbo();
}

void gbuffer::draw_geometry()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    unsigned attachments[] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(sizeof(attachments)/sizeof(unsigned), attachments);
    reinstate_current_fbo();
}

void gbuffer::draw_lighting()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    unsigned attachments[] = { GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(sizeof(attachments)/sizeof(unsigned), attachments);
    reinstate_current_fbo();
}
