/*
    Copyright 2018-2019 Julius Ikkala

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
#ifndef LT_SAMPLER_HH
#define LT_SAMPLER_HH
#include "api.hh"
#include "glheaders.hh"
#include "resource.hh"
#include "texture.hh"
#include "math.hh"

namespace lt
{

enum class interpolation: GLint
{
    LINEAR = GL_LINEAR,
    LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
    LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
    NEAREST = GL_NEAREST,
    NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
    NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
};
std::string to_string(interpolation i);

class LT_API sampler: public glresource
{
public:
    sampler(
        context& ctx,
        interpolation mag = interpolation::LINEAR,
        interpolation min = interpolation::LINEAR_MIPMAP_LINEAR,
        GLint extension = GL_REPEAT,
        unsigned anisotropy = 16,
        glm::vec4 border_color = glm::vec4(0,0,0,0),
        GLint comparison_mode = GL_NONE
    );
    ~sampler();

    void set_interpolation(
        interpolation mag,
        interpolation min = interpolation::LINEAR_MIPMAP_LINEAR
    );
    void set_extension(GLint extension);
    void set_anisotropy(unsigned anisotropy);
    void set_border_color(glm::vec4 border_color);
    void set_comparison_mode(GLint comparison_mode);

    // Returns the index
    GLint bind(const texture& tex, unsigned index = 0) const;

    // Returns the index
    GLint bind(
        GLuint tex,
        unsigned index = 0,
        GLenum target = GL_TEXTURE_2D
    ) const;

private:
    GLuint sampler_object;
};

} // namespace lt

#endif
