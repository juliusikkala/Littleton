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
#include "uniform.hh"
#include <stdexcept>
#include <cstring>

namespace lt
{

template<>
void uniform_set_value<float>(
    GLint location, size_t count, const float* value
){
    glUniform1fv(location, count, value);
}

template<>
void uniform_set_value<glm::vec2>(
    GLint location, size_t count, const glm::vec2* value
){
    glUniform2fv(location, count, (float*)value);
}

template<>
void uniform_set_value<glm::vec3>(
    GLint location, size_t count, const glm::vec3* value
){
    glUniform3fv(location, count, (float*)value);
}

template<>
void uniform_set_value<glm::vec4>(
    GLint location, size_t count, const glm::vec4* value
){
    glUniform4fv(location, count, (float*)value);
}

template<>
void uniform_set_value<int>(
    GLint location, size_t count, const int* value
){
    glUniform1iv(location, count, value);
}

template<>
void uniform_set_value<glm::ivec2>(
    GLint location, size_t count, const glm::ivec2* value
){
    glUniform2iv(location, count, (int*)value);
}

template<>
void uniform_set_value<glm::ivec3>(
    GLint location, size_t count, const glm::ivec3* value
){
    glUniform3iv(location, count, (int*)value);
}

template<>
void uniform_set_value<glm::ivec4>(
    GLint location, size_t count, const glm::ivec4* value
){
    glUniform4iv(location, count, (int*)value);
}

template<>
void uniform_set_value<unsigned>(
    GLint location, size_t count, const unsigned* value
){
    glUniform1uiv(location, count, value);
}

template<>
void uniform_set_value<glm::uvec2>(
    GLint location, size_t count, const glm::uvec2* value
){
    glUniform2uiv(location, count, (unsigned*)value);
}

template<>
void uniform_set_value<glm::uvec3>(
    GLint location, size_t count, const glm::uvec3* value
){
    glUniform3uiv(location, count, (unsigned*)value);
}

template<>
void uniform_set_value<glm::uvec4>(
    GLint location, size_t count, const glm::uvec4* value
){
    glUniform4uiv(location, count, (unsigned*)value);
}

// Bool vectors should be set using glm::ivec instead of this.
template<>
void uniform_set_value<bool>(
    GLint location, size_t count, const bool* value
){
    int v = *value;
    glUniform1iv(location, 1, &v);
}

template<>
void uniform_set_value<glm::mat2>(
    GLint location, size_t count, const glm::mat2* value
){
    glUniformMatrix2fv(location, count, GL_FALSE, (float*)value);
}

template<>
void uniform_set_value<glm::mat3>(
    GLint location, size_t count, const glm::mat3* value
){
    glUniformMatrix3fv(location, count, GL_FALSE, (float*)value);
}

template<>
void uniform_set_value<glm::mat4>(
    GLint location, size_t count, const glm::mat4* value
){
    glUniformMatrix4fv(location, count, GL_FALSE, (float*)value);
}

uniform_block_type::uniform_block_type(const uniform_block_type& other)
: uniforms(other.uniforms), size(other.size) { }

uniform_block_type::uniform_block_type(uniform_block_type&& other)
: uniforms(std::move(other.uniforms)), size(other.size) { }

uniform_block_type::uniform_block_type(
    std::unordered_map<std::string, uniform_info>&& uniforms,
    size_t size
): uniforms(std::move(uniforms)), size(size) { }

size_t uniform_block_type::get_total_size() const { return size; }

bool uniform_block_type::exists(const std::string& name) const
{
    auto it = uniforms.find(name);
    return it != uniforms.end();
}

uniform_block_type::uniform_info uniform_block_type::get_info(
    const std::string& name
) const
{
    auto it = uniforms.find(name);
    if(it == uniforms.end())
    {
        throw std::runtime_error("No uniform named \""+name+"\" in block");
    }
    return it->second;
}

bool uniform_block_type::operator==(const uniform_block_type& other) const
{
    return size == other.size && uniforms == other.uniforms;
}

bool uniform_block_type::operator!=(const uniform_block_type& other) const
{
    return size != other.size || uniforms != other.uniforms;
}

bool uniform_block_type::uniform_info::operator==(
    const uniform_info& other
) const
{
    return !memcmp(this, &other, sizeof(*this));
}

bool uniform_block_type::uniform_info::operator!=(
    const uniform_info& other
) const
{
    return memcmp(this, &other, sizeof(*this));
}

uniform_block::uniform_block(const uniform_block_type& type)
: type(type), ubo(0)
{
    basic_load();
}

uniform_block::uniform_block(const uniform_block& block)
: type(block.type), ubo(0)
{
    basic_load();
    memcpy(buffer, block.buffer, type.get_total_size());
}

uniform_block::uniform_block(uniform_block&& block)
: type(std::move(block.type)), ubo(block.ubo), buffer(block.buffer)
{
    block.ubo = 0;
    block.buffer = nullptr;
}

uniform_block::~uniform_block()
{
    basic_unload();
}

const uniform_block_type& uniform_block::get_type() const
{
    return type;
}

void uniform_block::bind(unsigned index) const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, index, ubo);
}

void uniform_block::upload()
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    memcpy(p, buffer, type.get_total_size());
    glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void uniform_block::basic_load()
{
    if(ubo != 0) return;

    buffer = new uint8_t[type.get_total_size()];

    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(
        GL_UNIFORM_BUFFER,
        type.get_total_size(),
        buffer,
        GL_DYNAMIC_DRAW
    );
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void uniform_block::basic_unload()
{
    if(ubo != 0)
    {
        glDeleteBuffers(1, &ubo);
        ubo = 0;
    }
    if(buffer != nullptr)
    {
        delete [] buffer;
        buffer = nullptr;
    }
}

} // namespace lt
