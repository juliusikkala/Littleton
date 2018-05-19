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
#ifndef LT_CONTEXT_HH
#define LT_CONTEXT_HH
#include "api.hh"
#include "glheaders.hh"
#include "math.hh"
#include <unordered_map>
#include <string>

namespace lt
{

class LT_API context
{
public:
    context();
    context(const context& other) = delete;
    context(context&& other) = delete;
    ~context();

    const std::string& get_vendor_name() const;
    const std::string& get_renderer() const;

    // Only single-value parameters supported through this function.
    GLint64 operator[](GLenum pname) const;

    GLint64 get(GLenum pname) const;
    glm::ivec2 get2(GLenum pname) const;
    glm::ivec3 get3(GLenum pname) const;
    glm::ivec4 get4(GLenum pname) const;

    GLint64 get(GLenum pname, GLuint index) const;
    glm::ivec2 get2(GLenum pname, GLuint index) const;
    glm::ivec3 get3(GLenum pname, GLuint index) const;
    glm::ivec4 get4(GLenum pname, GLuint index) const;

protected:
    void get(
        GLenum pname,
        size_t size,
        void* value,
        const GLuint* index = nullptr
    ) const;

    void init();

private:
    std::string vendor, renderer;

    mutable std::unordered_map<
        GLenum /*param*/,
        void*  /*value*/
    > param_cache;
};

} // namespace lt

#endif
