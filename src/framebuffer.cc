#include "framebuffer.hh"
#include "texture.hh"
#include "context.hh"
#include <stdexcept>
#include <string>

framebuffer::target_specifier::target_specifier(
    GLint format,
    bool as_texture
): format(format), as_texture(as_texture), use_texture(nullptr)
{
}

framebuffer::target_specifier::target_specifier(texture* use_texture)
: format(use_texture->get_internal_format()),
  as_texture(true), use_texture(use_texture)
{
}

framebuffer::framebuffer(
    context& ctx,
    glm::uvec2 size,
    const target_specification_map& target_specifications,
    unsigned samples
): render_target(ctx, size), target_specifications(target_specifications)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    std::vector<GLenum> attachments;

    for(auto& pair: target_specifications)
    {
        GLenum attachment = pair.first;
        const target_specifier& spec = pair.second;

        if(spec.as_texture)
        {
            texture* tex;

            if(spec.use_texture)
            {
                tex = spec.use_texture;
                if(tex->get_size() != size)
                {
                    throw std::runtime_error(
                        "Texture size does not match framebuffer size!"
                    );
                }
            }
            else
            {
                tex = new texture(
                    ctx,
                    size,
                    spec.format,
                    GL_UNSIGNED_BYTE,
                    // TODO: Introduce samplers so that irrelevant stuff is not
                    // set here.
                    texture::params(
                        false,
                        GL_LINEAR,
                        GL_CLAMP_TO_EDGE,
                        0,
                        glm::vec4(0),
                        samples
                    )
                );
                owned_textures.emplace_back(tex);
            }

            glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                attachment,
                tex->get_target(),
                tex->get_texture(),
                0
            );

            targets.emplace(attachment, tex);
        }
        else
        {
            GLuint rbo;
            glGenRenderbuffers(1, &rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);

            if(samples)
            {
                glRenderbufferStorageMultisample(
                    GL_RENDERBUFFER,
                    samples,
                    spec.format,
                    size.x,
                    size.y
                );
            }
            else
            {
                glRenderbufferStorage(
                    GL_RENDERBUFFER,
                    spec.format,
                    size.x,
                    size.y
                );
            }

            if(glGetError() != GL_NO_ERROR)
                throw std::runtime_error("Failed to create render buffer");

            glFramebufferRenderbuffer(
                GL_FRAMEBUFFER,
                attachment,
                GL_RENDERBUFFER,
                rbo
            );

            targets.emplace(attachment, rbo);
        }

        if(
            attachment != GL_DEPTH_ATTACHMENT &&
            attachment != GL_DEPTH_STENCIL_ATTACHMENT
        ) attachments.push_back(attachment);
    }

    glDrawBuffers(attachments.size(), attachments.data());

    if(glGetError() != GL_NO_ERROR)
        throw std::runtime_error("Failed to create framebuffer");

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Created framebuffer is incomplete!");

    reinstate_current_fbo();
}

framebuffer::framebuffer(framebuffer&& f)
: render_target(f), target_specifications(std::move(f.target_specifications)),
  owned_textures(std::move(f.owned_textures)), targets(std::move(f.targets))
{
    f.fbo = 0;
    f.targets.clear();
}

framebuffer::~framebuffer()
{
    for(const auto& pair: targets)
    {
        if(auto rbo = std::get_if<GLuint>(&pair.second))
        {
            if(*rbo != 0) glDeleteRenderbuffers(1, rbo);
        }
    }

    if(fbo != 0) glDeleteFramebuffers(1, &fbo);
}

texture* framebuffer::get_texture_target(GLenum attachment) const
{
    auto it = targets.find(attachment);
    if(it == targets.end())
    {
        throw std::runtime_error(
            "Framebuffer does not contain requested attachment!"
        );
    }

    if(auto tex = std::get_if<texture*>(&it->second))
    {
        return *tex;
    }
    throw std::runtime_error(
        "Requested attachment is a render buffer object, not a texture"
    );
}

