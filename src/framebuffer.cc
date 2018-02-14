#include "framebuffer.hh"
#include "texture.hh"
#include "context.hh"
#include <stdexcept>
#include <string>

framebuffer::framebuffer(
    context& ctx,
    glm::uvec2 size,
    std::vector<texture*>&& targets,
    GLenum depth_stencil_format
): render_target(ctx, size), color_targets(std::move(targets)),
   depth_stencil_target(nullptr)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if(depth_stencil_format)
    {
        glGenRenderbuffers(1, &depth_stencil_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_rbo);
        glRenderbufferStorage(
            GL_RENDERBUFFER,
            depth_stencil_format,
            size.x,
            size.y
        );
    }
    else depth_stencil_rbo = 0;

    reinstate_current_fbo();
}

framebuffer::framebuffer(
    context& ctx,
    glm::uvec2 size,
    std::vector<texture*>&& targets,
    texture* depth_stencil_target
): render_target(ctx, size), color_targets(std::move(targets)),
   depth_stencil_rbo(0)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    this->depth_stencil_target = depth_stencil_target;

    reinstate_current_fbo();
}

framebuffer::framebuffer(framebuffer&& f)
: render_target(f), color_targets(std::move(f.color_targets)),
  depth_stencil_target(f.depth_stencil_target),
  depth_stencil_rbo(f.depth_stencil_rbo)
{
    f.fbo = 0;
    f.depth_stencil_rbo = 0;
}

framebuffer::~framebuffer()
{
    if(depth_stencil_rbo != 0) glDeleteRenderbuffers(1, &depth_stencil_rbo);
    if(fbo != 0) glDeleteFramebuffers(1, &fbo);
}

void framebuffer::set_target(texture* target, unsigned index)
{
    unsigned max_attachments = get_context()[GL_MAX_COLOR_ATTACHMENTS];
    if(index >= max_attachments)
        throw std::runtime_error(
            "Unable to set framebuffer target at index "
            + std::to_string(index) + ", maximum is "
            + std::to_string(max_attachments-1)
        );

    if(target->get_size() != size)
        throw std::runtime_error(
            "Can't bind texture of size "
            + std::to_string(target->get_size().x) + ", "
            + std::to_string(target->get_size().y) + " to framebuffer of size "
            + std::to_string(size.x) + ", "
            + std::to_string(size.y) + "."
        );

    if(color_targets.size() <= index)
    {
        color_targets.resize(index + 1, nullptr);
    }
    color_targets[index] = target;
}

void framebuffer::remove_target(unsigned index)
{
    if(color_targets.size() > index)
    {
        color_targets[index] = nullptr;
        while(color_targets.size() && color_targets.back() == nullptr)
            color_targets.pop_back();
    }
}

void framebuffer::set_depth_target(texture* target)
{
    if(target->get_size() != size)
        throw std::runtime_error(
            "Can't bind texture of size "
            + std::to_string(target->get_size().x) + ", "
            + std::to_string(target->get_size().y) + " to framebuffer of size "
            + std::to_string(size.x) + ", "
            + std::to_string(size.y) + "."
        );

    depth_stencil_target = target;
}

void framebuffer::remove_depth_target()
{
    depth_stencil_target = nullptr;
}

void framebuffer::bind(GLenum target)
{
    render_target::bind(target);
    std::vector<unsigned> attachments;
    for(unsigned i = 0; i < color_targets.size(); ++i)
    {
        texture* t = color_targets[i];
        glFramebufferTexture2D(
            target,
            GL_COLOR_ATTACHMENT0+i,
            t->get_target(),
            t->get_texture(),
            0
        );
        attachments.push_back(GL_COLOR_ATTACHMENT0+i);
    }
    if(depth_stencil_target)
    {
        glFramebufferTexture2D(
            target,
            GL_DEPTH_STENCIL_ATTACHMENT,
            depth_stencil_target->get_target(),
            depth_stencil_target->get_texture(),
            0
        );
    }
    else if(depth_stencil_rbo)
    {
        glFramebufferRenderbuffer(
            target,
            GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER,
            depth_stencil_rbo
        );
    }
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Attempt to bind an incomplete framebuffer!");

    if(target == GL_DRAW_FRAMEBUFFER || target == GL_FRAMEBUFFER)
        glDrawBuffers(attachments.size(), attachments.data());

    if(target == GL_READ_FRAMEBUFFER || target == GL_FRAMEBUFFER)
        glReadBuffer(GL_COLOR_ATTACHMENT0);
}
