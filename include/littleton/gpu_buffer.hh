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
#ifndef LT_GPU_BUFFER_HH
#define LT_GPU_BUFFER_HH
#include "api.hh"
#include "glheaders.hh"
#include "resource.hh"

namespace lt
{

class LT_API gpu_buffer: public resource, public glresource
{
public:
    explicit gpu_buffer(context& ctx);
    gpu_buffer(
        context& ctx,
        GLenum target,
        size_t size,
        const void* data,
        GLenum usage = GL_STATIC_DRAW
    );
    gpu_buffer(gpu_buffer&& other);
    ~gpu_buffer();

    GLuint get_buffer() const;
    GLenum get_target() const;
    size_t get_size() const;

    void bind() const;
    // Only useful if you need to bind several at once for some reason
    void bind(unsigned index) const;

    // Creates a lazily loaded buffer. Takes ownership of the pointers.
    static gpu_buffer* create(
        context& ctx,
        GLenum target,
        size_t size,
        const void* data,
        GLenum usage = GL_STATIC_DRAW
    );

protected:
    void basic_load(
        GLenum target,
        size_t size,
        const void* data,
        GLenum usage
    ) const;

    void basic_unload() const;

    mutable GLuint buf;
    mutable GLenum target;
    mutable size_t size;
};

} // namespace lt

#endif

