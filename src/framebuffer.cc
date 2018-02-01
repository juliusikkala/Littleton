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
): render_target(ctx, fbo, size), targets(std::move(targets))
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if(depth_stencil_format)
    {
        glGenRenderbuffers(1, &depth_stencil);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil);
        glRenderbufferStorage(
            GL_RENDERBUFFER,
            depth_stencil_format,
            size.x,
            size.y
        );
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER,
            depth_stencil
        );
    }
    else depth_stencil = 0;

    if(current_fbo != -1) glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);
}

framebuffer::framebuffer(framebuffer&& f)
: render_target(f), targets(std::move(f.targets)),
  depth_stencil(f.depth_stencil)
{
    f.fbo = 0;
    f.depth_stencil = 0;
}

framebuffer::~framebuffer()
{
    if(depth_stencil != 0) glDeleteRenderbuffers(1, &depth_stencil);
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

    if(targets.size() <= index)
    {
        targets.resize(index + 1, nullptr);
    }
    targets[index] = target;
}

void framebuffer::remove_target(unsigned index)
{
    if(targets.size() > index)
    {
        targets[index] = nullptr;
        while(targets.size() && targets.back() == nullptr) targets.pop_back();
    }
}

void framebuffer::bind()
{
    render_target::bind();
    for(unsigned i = 0; i < targets.size(); ++i)
    {
        texture* target = targets[i];
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0+i,
            target->get_target(),
            target->get_texture(),
            0
        );
    }
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Attempt to bind an incomplete framebuffer!");
}
