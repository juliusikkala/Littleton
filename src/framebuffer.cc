#include "framebuffer.hh"
#include "texture.hh"
#include "context.hh"
#include <stdexcept>
#include <string>

framebuffer::framebuffer(
    context& ctx,
    glm::uvec2 size,
    std::vector<texture*>&& targets,
    GLenum depth_stencil_format,
    unsigned render_buffer_samples
): render_target(ctx, size), color_targets(std::move(targets)),
   depth_stencil_target(nullptr)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if(depth_stencil_format)
    {
        glGenRenderbuffers(1, &depth_stencil_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_rbo);

        switch(depth_stencil_format)
        {
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
        case GL_DEPTH_COMPONENT32F:
            depth_stencil_attachment = GL_DEPTH_ATTACHMENT;
            break;
        default:
            depth_stencil_attachment = GL_DEPTH_STENCIL_ATTACHMENT;
            break;
        }

        if(render_buffer_samples)
        {
            glRenderbufferStorageMultisample(
                GL_RENDERBUFFER,
                render_buffer_samples,
                depth_stencil_format,
                size.x,
                size.y
            );
        }
        else
        {
            glRenderbufferStorage(
                GL_RENDERBUFFER,
                depth_stencil_format,
                size.x,
                size.y
            );
        }

        if(glGetError() != GL_NO_ERROR)
            throw std::runtime_error("Failed to create render buffer");
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

    reinstate_current_fbo();

    set_depth_target(depth_stencil_target);
}

framebuffer::framebuffer(framebuffer&& f)
: render_target(f), color_targets(std::move(f.color_targets)),
  depth_stencil_target(f.depth_stencil_target),
  depth_stencil_rbo(f.depth_stencil_rbo),
  depth_stencil_attachment(f.depth_stencil_attachment)
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
    depth_stencil_attachment = 
        target->get_external_format() == GL_DEPTH_STENCIL ?
            GL_DEPTH_ATTACHMENT : GL_DEPTH_STENCIL_ATTACHMENT;
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
            depth_stencil_attachment,
            depth_stencil_target->get_target(),
            depth_stencil_target->get_texture(),
            0
        );
    }
    else if(depth_stencil_rbo)
    {
        glFramebufferRenderbuffer(
            target,
            depth_stencil_attachment,
            GL_RENDERBUFFER,
            depth_stencil_rbo
        );
    }

    if(target == GL_DRAW_FRAMEBUFFER || target == GL_FRAMEBUFFER)
    {
        if(attachments.size() > 0)
            glDrawBuffers(attachments.size(), attachments.data());
        else glDrawBuffer(GL_NONE);
    }

    if(target == GL_READ_FRAMEBUFFER || target == GL_FRAMEBUFFER)
    {
        if(attachments.size() > 0)
            glReadBuffer(GL_COLOR_ATTACHMENT0);
    }

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Attempt to bind an incomplete framebuffer!");
}
