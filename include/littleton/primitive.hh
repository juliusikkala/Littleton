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
#ifndef LT_PRIMITIVE_HH
#define LT_PRIMITIVE_HH
#include "glheaders.hh"
#include "resource.hh"
#include "shader.hh"
#include "gpu_buffer.hh"
#include <map>

namespace lt
{

class gpu_buffer_accessor
{
friend class primitive;
public:
    gpu_buffer_accessor();

    gpu_buffer_accessor(
        const gpu_buffer& buf,
        unsigned components = 4,
        GLenum type = GL_FLOAT,
        bool normalized = false,
        size_t stride = 0,
        size_t offset = 0
    );
    ~gpu_buffer_accessor();

    bool is_valid() const;
    size_t get_element_count() const;

private:
    void setup_vertex_attrib(unsigned index) const;
    
    const gpu_buffer* buf;
    unsigned components;
    GLenum type;
    bool normalized;
    size_t stride;
    size_t offset;
};

class primitive: public resource, public glresource
{
public:
    struct attribute
    {
        unsigned index;
        std::string name;

        bool operator<(const attribute& other) const;
    };

    static const attribute POSITION;
    static const attribute NORMAL;
    static const attribute TANGENT;
    static const attribute UV0;
    static const attribute UV1;
    static const attribute UV2;
    static const attribute UV3;
    static constexpr unsigned USER_INDEX = 7;

    explicit primitive(context& ctx);
    primitive(
        context& ctx,
        size_t index_count,
        GLenum mode,
        const gpu_buffer_accessor& index,
        const std::map<attribute, gpu_buffer_accessor>& attributes
    );
    primitive(primitive&& other);
    ~primitive();

    void update_definitions(shader::definition_map& def) const;

    GLuint get_vao() const;
    void draw() const;
    GLenum get_mode() const;

    // Creates a lazily loaded buffer. Takes ownership of the pointers.
    static primitive* create(
        context& ctx,
        size_t index_count,
        GLenum mode,
        const gpu_buffer_accessor& index,
        const std::map<attribute, gpu_buffer_accessor>& attributes
    );

protected:
    void basic_load(
        size_t index_count,
        GLenum mode,
        const gpu_buffer_accessor& index,
        const std::map<attribute, gpu_buffer_accessor>& attributes
    ) const;

    void basic_unload() const;

    mutable GLuint vao;
    mutable size_t index_count;
    mutable GLenum mode;
    mutable gpu_buffer_accessor index;
    mutable std::map<attribute, gpu_buffer_accessor> attribs;
};

} // namespace lt

#endif
