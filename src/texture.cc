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
#include "texture.hh"
#include "context.hh"
#include "helpers.hh"
#include "stb_image.h"
#include <algorithm>
#include <cstring>

namespace
{
using namespace lt;

unsigned choose_alignment(unsigned line_bytes)
{
    if((line_bytes & 7) == 0) return 8;
    if((line_bytes & 3) == 0) return 4;
    if((line_bytes & 1) == 0) return 2;
    return 1;
}

GLuint create_texture_from_data(
    GLenum target,
    GLenum type,
    glm::uvec3 dims,
    GLint internal_format,
    const void* data,
    unsigned samples = 0
){
    unsigned mipmap_count = calculate_mipmap_count(glm::uvec2(dims));

    GLint external_format = internal_format_to_external_format(internal_format);

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    GLint prev_tex = 0;
    glGetIntegerv(get_binding_name(target), &prev_tex);
    glBindTexture(target, tex);

    unsigned n = internal_format_channel_count(internal_format);
    glPixelStorei(GL_UNPACK_ALIGNMENT, choose_alignment(dims.x * n));

    switch(target)
    {
    case GL_TEXTURE_2D:
        glTexStorage2D(target, mipmap_count, internal_format, dims.x, dims.y);

        if(data)
        {
            glTexSubImage2D(
                target, 0, 0, 0, dims.x, dims.y,
                external_format, type,
                data
            );
            glGenerateMipmap(target);
        }
        break;
    case GL_TEXTURE_1D:
        glTexStorage1D(target, mipmap_count, internal_format, dims.x);

        if(data)
        {
            glTexSubImage1D(
                target, 0, 0, dims.x,
                external_format, type,
                data
            );

            glGenerateMipmap(target);
        }
        break;
    case GL_TEXTURE_2D_MULTISAMPLE:
        glTexImage2DMultisample(
            target,
            samples,
            internal_format,
            dims.x,
            dims.y,
            true
        );
        break;
    case GL_TEXTURE_CUBE_MAP:
        glTexStorage2D(target, mipmap_count, internal_format, dims.x, dims.y);

        if(data)
        {
            size_t face_bytes = n * gl_type_sizeof(type) * dims.x * dims.y;
            for(unsigned i = 0; i < 6; ++i)
            {
                GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
                glTexSubImage2D(
                    face, 0, 0, 0, dims.x, dims.y,
                    external_format, type,
                    ((uint8_t*)data) + face_bytes * i
                );
            }
        }
        glGenerateMipmap(target);
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY:
        glTexStorage3D(
            target,
            mipmap_count,
            internal_format,
            dims.x,
            dims.y,
            dims.z*6 // face-layers instead of just layers
        );

        if(data)
        {
            size_t face_layer_bytes =
                n * gl_type_sizeof(type) * dims.x * dims.y;

            for(unsigned layer = 0; layer < dims.z; ++layer)
            {
                for(unsigned i = 0; i < 6; ++i)
                {
                    GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
                    glTexSubImage3D(
                        face, 0, 0, 0, layer*6 + i, dims.x, dims.y, 1,
                        external_format, type,
                        ((uint8_t*)data) + face_layer_bytes * (layer * 6 + i)
                    );
                }
            }
        }
        glGenerateMipmap(target);
        break;
    default:
        throw std::runtime_error("Unknown texture target!");
    }

    if(prev_tex != 0) glBindTexture(target, prev_tex);

    return tex;
}

GLuint load_texture(
    context& ctx,
    const std::string& path,
    bool srgb,
    GLenum target,
    GLint& internal_format,
    GLenum& type,
    glm::uvec3& dimensions
){
    int w = 0, h = 0, n = 0;
    bool hdr = stbi_is_hdr(path.c_str());
    void* data = nullptr;

    stbi_set_flip_vertically_on_load(target != GL_TEXTURE_CUBE_MAP);

    if(hdr)
    {
        data = stbi_loadf(path.c_str(), &w, &h, &n, 0);
        type = GL_FLOAT;
    }
    else
    {
        data = stbi_load(path.c_str(), &w, &h, &n, 0);
        type = GL_UNSIGNED_BYTE;
    }
    dimensions.x = w;
    dimensions.y = h;
    dimensions.z = 1;

    if(target == GL_TEXTURE_CUBE_MAP)
        dimensions.y /= 6;

    if(!data)
    {
        throw std::runtime_error("Unable to read " + path);
    }
    int max_size = ctx[GL_MAX_TEXTURE_SIZE];
    if(w > max_size || h > max_size)
        throw std::runtime_error(
            "Texture is too large, maximum is "
            + std::to_string(max_size) + "x" + std::to_string(max_size)
            + " but texture is " + std::to_string(w) + "x"
            + std::to_string(h) + "."
        );

    switch(n)
    {
    case 1:
        internal_format = hdr ? GL_R16F : GL_R8;
        break;
    case 2:
        internal_format = hdr ? GL_RG16F : GL_RG8;
        break;
    case 3:
        if(srgb) internal_format = GL_SRGB8;
        else internal_format = hdr ? GL_RGB16F : GL_RGB8;
        break;
    case 4:
        if(srgb) internal_format = GL_SRGB8_ALPHA8;
        else internal_format = hdr ? GL_RGBA16F : GL_RGBA8;
        break;
    }

    GLuint tex = create_texture_from_data(
        target,
        type,
        dimensions,
        internal_format,
        data
    );

    if(glGetError() != GL_NO_ERROR)
        throw std::runtime_error("Failed to create texture from " + path);

    stbi_image_free(data);
    return tex;
}

}

namespace lt
{

texture::texture(context& ctx)
: glresource(ctx), tex(0), target(GL_TEXTURE_2D) {}

texture::texture(
    context& ctx,
    const std::string& path,
    bool srgb,
    GLenum target
): glresource(ctx), tex(0), target(target)
{
    basic_load(path, srgb, target);
}

texture::texture(
    context& ctx,
    glm::uvec2 size,
    GLint internal_format,
    GLenum type,
    unsigned samples,
    GLenum target,
    const void* data
):  texture(
        ctx,
        glm::uvec3(size, 1),
        internal_format,
        type,
        samples,
        target,
        data
    )
{}

texture::texture(
    context& ctx,
    glm::uvec3 dimensions,
    GLint internal_format,
    GLenum type,
    unsigned samples,
    GLenum target,
    const void* data
): glresource(ctx), tex(0), internal_format(internal_format), target(target),
   type(type)
{
    basic_load(
        dimensions,
        internal_format,
        type,
        samples,
        target,
        data
    );
}

texture::texture(texture&& other)
: glresource(other.get_context())
{
    other.load();

    tex = other.tex;
    internal_format = other.internal_format;
    target = other.target;
    type = other.type;
    dimensions = other.dimensions;

    other.tex = 0;
}

texture::~texture()
{
    basic_unload();
}

GLuint texture::get_texture() const
{
    load();
    return tex;
}

GLint texture::get_internal_format() const
{
    load();
    return internal_format;
}

GLenum texture::get_external_format() const
{
    load();
    return internal_format_to_external_format(internal_format);
}

GLenum texture::get_target() const
{
    load();
    return target;
}

GLenum texture::get_type() const
{
    load();
    return type;
}

glm::uvec2 texture::get_size() const
{
    load();
    return glm::uvec2(dimensions);
}

glm::uvec3 texture::get_dimensions() const
{
    load();
    return dimensions;
}

void texture::generate_mipmaps()
{
    load();
    glBindTexture(target, tex);
    glGenerateMipmap(target);
}

class file_texture: public texture
{
public:
    file_texture(
        context& ctx,
        const std::string& path,
        bool srgb,
        GLenum target
    ): texture(ctx), srgb(srgb), path(path)
    {
        this->target = target;
    }

protected:
    void load_impl() const override
    {
        basic_load(path, srgb, target);
    }

    void unload_impl() const override
    {
        basic_unload();
    }

private:
    bool srgb;
    std::string path;
};

texture* texture::create(
    context& ctx,
    const std::string& path,
    bool srgb,
    GLenum target
){
    return new file_texture(ctx, path, srgb, target);
}

class data_texture: public texture
{
public:
    data_texture(
        context& ctx,
        glm::uvec3 dimensions,
        GLint internal_format,
        GLenum type,
        unsigned samples,
        GLenum target,
        size_t data_size,
        const void* data
    ): texture(ctx), samples(samples), dimensions(dimensions)
    {
        this->internal_format = internal_format;
        this->type = type;
        this->target = target;

        if(data)
        {
            this->data = new uint8_t[data_size];
            memcpy(this->data, data, data_size);
        }
    }

    ~data_texture()
    {
        if(data) delete [] (uint8_t*)data;
    }

protected:
    void load_impl() const override
    {
        basic_load(
            dimensions,
            internal_format,
            type,
            samples,
            target,
            data
        );
    }

    void unload_impl() const override
    {
        basic_unload();
    }

private:
    unsigned samples;
    glm::uvec3 dimensions;
    void* data;
};

texture* texture::create(
    context& ctx,
    glm::uvec2 size,
    GLint internal_format,
    GLenum type,
    unsigned samples,
    GLenum target,
    size_t data_size,
    const void* data
){
    return new data_texture(
        ctx, glm::uvec3(size, 1), internal_format, type, samples, target,
        data_size, data
    );
}

texture* texture::create(
    context& ctx,
    glm::uvec3 dimensions,
    GLint internal_format,
    GLenum type,
    unsigned samples,
    GLenum target,
    size_t data_size,
    const void* data
){
    return new data_texture(
        ctx, dimensions, internal_format, type, samples, target,
        data_size, data
    );
}

void texture::basic_load(
    const std::string& path,
    bool srgb,
    GLenum target
) const
{
    if(tex) return;

    tex = load_texture(
        get_context(),
        path,
        srgb,
        target,
        internal_format,
        type,
        dimensions
    );

    if(tex == 0)
        throw std::runtime_error("Unable to read texture " + path);

    this->target = target;
}

void texture::basic_load(
    glm::uvec3 dimensions,
    GLint internal_format,
    GLenum type,
    unsigned samples,
    GLenum target,
    const void* data
) const {
    if(tex) return;

    tex = create_texture_from_data(
        target,
        type,
        dimensions,
        internal_format,
        data,
        samples
    );

    if(glGetError() != GL_NO_ERROR)
        throw std::runtime_error("Failed to create a texture from data");

    this->internal_format = internal_format;
    this->type = type;
    this->target = target;
    this->dimensions = dimensions;
}

void texture::basic_unload() const
{
    if(tex != 0)
    {
        glDeleteTextures(1, &tex);
        tex = 0;
    }
}

} // namespace lt
