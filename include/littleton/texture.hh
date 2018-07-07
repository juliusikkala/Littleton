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
#ifndef LT_TEXTURE_HH
#define LT_TEXTURE_HH
#include "api.hh"
#include "glheaders.hh"
#include "resource.hh"
#include "math.hh"
#include <string>

namespace lt
{

class LT_API texture: public resource, public glresource
{
public:
    texture(
        context& ctx,
        const std::string& path,
        bool srgb = false,
        GLenum target = GL_TEXTURE_2D
    );

    texture(
        context& ctx,
        glm::uvec2 size,
        GLint format,
        GLenum type,
        unsigned samples = 0,
        GLenum target = GL_TEXTURE_2D,
        const void* data = nullptr
    );

    texture(
        context& ctx,
        glm::uvec3 dimensions,
        GLint format,
        GLenum type,
        unsigned samples = 0,
        GLenum target = GL_TEXTURE_2D_ARRAY,
        const void* data = nullptr
    );

    texture(const texture& other) = delete;
    texture(texture&& other);
    ~texture();

    GLuint get_texture() const;
    GLint get_internal_format() const;
    GLenum get_external_format() const;
    GLenum get_target() const;
    GLenum get_type() const;
    glm::uvec2 get_size() const;
    glm::uvec3 get_dimensions() const;
    size_t get_texel_size() const;

    void generate_mipmaps();

    static texture* create(
        context& ctx,
        const std::string& path,
        bool srgb = false,
        GLenum target = GL_TEXTURE_2D
    );

    static texture* create(
        context& ctx,
        glm::uvec2 size,
        GLint internal_format,
        GLenum type,
        unsigned samples = 0,
        GLenum target = GL_TEXTURE_2D,
        size_t data_size = 0,
        const void* data = nullptr
    );

    static texture* create(
        context& ctx,
        glm::uvec3 dimensions,
        GLint internal_format,
        GLenum type,
        unsigned samples = 0,
        GLenum target = GL_TEXTURE_2D_ARRAY,
        size_t data_size = 0,
        const void* data = nullptr
    );

    template<typename T>
    std::vector<T> read() const;

protected:
    explicit texture(context& ctx);

    void basic_load(
        const std::string& path,
        bool srgb,
        GLenum target
    ) const;

    void basic_load(
        glm::uvec3 dimensions,
        GLint internal_format,
        GLenum type,
        unsigned samples,
        GLenum target,
        const void* data = nullptr
    ) const;

    void basic_unload() const;

    mutable GLuint tex;
    mutable GLint internal_format;
    mutable GLint target;
    mutable GLenum type;
    mutable glm::uvec3 dimensions;
};

} // namespace lt

#include "texture.tcc"

#endif
