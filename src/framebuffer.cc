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
#include "framebuffer.hh"
#include "texture.hh"
#include "context.hh"
#include "helpers.hh"
#include <stdexcept>
#include <string>
#include <boost/functional/hash.hpp>

namespace lt
{

framebuffer::target_specifier::target_specifier(
    GLint format,
    bool as_texture
): format(format), as_texture(as_texture), use_texture(nullptr)
{
}

bool framebuffer::target_specifier::operator==(
    const target_specifier& other
) const
{
    return other.format == format && other.as_texture == as_texture
        && other.use_texture == use_texture;
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
    unsigned samples,
    GLenum target
):  framebuffer(
        ctx,
        glm::uvec3(size, 1),
        target_specifications,
        samples,
        target
    )
{}

framebuffer::framebuffer(
    context& ctx,
    glm::uvec3 dimensions,
    const target_specification_map& target_specifications,
    unsigned samples,
    GLenum target
):  render_target(ctx, target, dimensions),
    target_specifications(target_specifications), samples(samples),
    target(target)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    std::vector<GLenum> attachments;

    for(auto& pair: target_specifications)
    {
        GLenum attachment = pair.first;
        const target_specifier& spec = pair.second;

        if(
            spec.as_texture ||
            target == GL_TEXTURE_CUBE_MAP ||
            target == GL_TEXTURE_CUBE_MAP_ARRAY
        ){
            texture* tex;

            if(spec.use_texture)
            {
                tex = spec.use_texture;
                if(tex->get_dimensions() != dimensions)
                {
                    throw std::runtime_error(
                        "Texture dimensions do not match framebuffer "
                        "dimensions!"
                    );
                }
            }
            else
            {
                tex = new texture(
                    ctx,
                    dimensions,
                    spec.format,
                    internal_format_compatible_type(spec.format),
                    samples,
                    target
                );
                owned_textures.emplace_back(tex);
            }

            glFramebufferTexture(
                GL_FRAMEBUFFER,
                attachment,
                tex->get_texture(),
                0
            );

            targets.emplace(attachment, tex);
            if(glGetError() != GL_NO_ERROR)
                throw std::runtime_error("Failed to create texture target");
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
                    dimensions.x,
                    dimensions.y
                );
            }
            else
            {
                glRenderbufferStorage(
                    GL_RENDERBUFFER,
                    spec.format,
                    dimensions.x,
                    dimensions.y
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
:   render_target(f), target_specifications(std::move(f.target_specifications)),
    samples(f.samples), target(f.target),
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

const framebuffer::target_specification_map&
framebuffer::get_target_specifications() const
{
    return target_specifications;
}
unsigned framebuffer::get_samples() const { return samples; }
GLenum framebuffer::get_target() const { return target; }

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

} // namespace lt

size_t boost::hash_value(const lt::framebuffer::target_specifier& t)
{
    std::size_t seed = 0;
    boost::hash_combine(seed, t.format);
    boost::hash_combine(seed, t.as_texture);
    boost::hash_combine(seed, t.use_texture);
    return seed;
}
