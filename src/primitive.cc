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
#include "primitive.hh"
#include <stdexcept>
#include <string>

namespace lt
{

gpu_buffer_accessor::gpu_buffer_accessor()
:   buf(nullptr), components(0), type(GL_INT), normalized(false),
    stride(0), offset(0)
{
}

gpu_buffer_accessor::gpu_buffer_accessor(
    const gpu_buffer& buf,
    unsigned components,
    GLenum type,
    bool normalized,
    size_t stride,
    size_t offset
):  buf(&buf), components(components), type(type), normalized(normalized),
    stride(stride), offset(offset)
{}

gpu_buffer_accessor::~gpu_buffer_accessor()
{}

bool gpu_buffer_accessor::is_valid() const
{
    return buf != nullptr;
}

size_t gpu_buffer_accessor::get_element_count() const
{
    if(!buf) return 0;
    size_t step = (stride == 0 ? components : stride);
    return buf->get_size()/step;
}

void gpu_buffer_accessor::setup_vertex_attrib(unsigned i) const
{
    if(!is_valid() || buf->get_target() != GL_ARRAY_BUFFER)
    {
        glDisableVertexAttribArray(i);
        return;
    }

    buf->bind();
    glVertexAttribPointer(
        i,
        components,
        type,
        normalized,
        stride,
        (const GLvoid*)(offset)
    );
    glEnableVertexAttribArray(i);
}

bool primitive::attribute::operator<(const attribute& other) const
{
    return index < other.index;
}

const primitive::attribute primitive::POSITION = {0, "VERTEX_POSITION"};
const primitive::attribute primitive::NORMAL = {1, "VERTEX_NORMAL"};
const primitive::attribute primitive::TANGENT = {2, "VERTEX_TANGENT"};
const primitive::attribute primitive::UV0 = {3, "VERTEX_UV0"};
const primitive::attribute primitive::UV1 = {4, "VERTEX_UV1"};
const primitive::attribute primitive::UV2 = {5, "VERTEX_UV2"};
const primitive::attribute primitive::UV3 = {6, "VERTEX_UV3"};

primitive::primitive(context& ctx)
: glresource(ctx), vao(0) {}

primitive::primitive(
    context& ctx,
    size_t index_count,
    GLenum mode,
    const gpu_buffer_accessor& index,
    const std::map<attribute, gpu_buffer_accessor>& attributes
): glresource(ctx), vao(0)
{
    basic_load(index_count, mode, index, attributes);
}

primitive::primitive(primitive&& other)
: glresource(other.get_context())
{
    other.load();

    vao = other.vao;
    index_count = other.index_count;
    mode = other.mode;
    index = other.index;
    attribs = other.attribs;

    other.vao = 0;
}

primitive::~primitive()
{
    basic_unload();
}

void primitive::update_definitions(shader::definition_map& def) const
{
    load();

    for(const auto& pair: attribs)
        def[pair.first.name] = std::to_string(pair.first.index);
}

GLuint primitive::get_vao() const
{
    load();
    return vao;
}

void primitive::draw() const
{
    load();
    glBindVertexArray(vao);
    if(index.is_valid())
        glDrawElements(
            mode,
            index_count,
            index.type,
            (const GLvoid*)index.offset
        );
    else
        glDrawArrays(
            mode,
            0,
            index_count
        );

    glBindVertexArray(0);
}

GLenum primitive::get_mode() const
{
    return mode;
}

class lazy_primitive: public primitive
{
public:
    lazy_primitive(
        context& ctx,
        size_t index_count,
        GLenum mode,
        const gpu_buffer_accessor& index,
        const std::map<attribute, gpu_buffer_accessor>& attributes
    ):  primitive(ctx)
    {
        this->index_count= index_count;
        this->mode = mode;
        this->index = index;
        this->attribs = attributes;
    }

    ~lazy_primitive() {}

protected:
    void load_impl() const override
    {
        basic_load(index_count, mode, index, attribs);
    }

    void unload_impl() const override
    {
        basic_unload();
    }
};

primitive* primitive::create(
    context& ctx,
    size_t index_count,
    GLenum mode,
    const gpu_buffer_accessor& index,
    const std::map<attribute, gpu_buffer_accessor>& attributes
){
    return new lazy_primitive(ctx, index_count, mode, index, attributes);
}

void primitive::basic_load(
    size_t index_count,
    GLenum mode,
    const gpu_buffer_accessor& index,
    const std::map<attribute, gpu_buffer_accessor>& attributes
) const
{
    if(vao) return;

    if(index.is_valid() && index.buf->get_target() != GL_ELEMENT_ARRAY_BUFFER)
        throw std::runtime_error(
            "Attempt to load index buffer with target "
            + std::to_string(index.buf->get_target())
        );
    for(const auto& pair: attribs)
        if(pair.second.buf->get_target() != GL_ARRAY_BUFFER)
            throw std::runtime_error(
                "Attempt to load vertex buffer with target "
                + std::to_string(pair.second.buf->get_target())
            );

    this->index = index;
    this->attribs = attributes;
    this->index_count = index_count;
    this->mode = mode;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    if(index.is_valid())
    {
        index.buf->link();
        index.buf->bind();
    }
    for(auto& pair: attribs)
    {
        pair.second.buf->link();
        pair.second.setup_vertex_attrib(pair.first.index);
    }
    glBindVertexArray(0);

    if(glGetError() != GL_NO_ERROR)
        throw std::runtime_error("Failed to create a primitive");
}

void primitive::basic_unload() const
{
    if(vao != 0)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;

        if(index.is_valid())
            index.buf->unlink();

        for(const auto& pair: attribs)
            pair.second.buf->unlink();
    }
}

} // namespace lt
