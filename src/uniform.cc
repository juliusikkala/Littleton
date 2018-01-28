#include "uniform.hh"
#include <stdexcept>
#include <cstring>

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
: info(other.info), size(other.size) { }

uniform_block_type::uniform_block_type(uniform_block_type&& other)
: info(std::move(other.info)), size(other.size) { }

uniform_block_type::uniform_block_type(
    std::unordered_map<std::string, uniform_info>&& info,
    size_t size
): info(std::move(info)), size(size) { }

size_t uniform_block_type::get_total_size() const { return size; }

bool uniform_block_type::exists(const std::string& name) const
{
    auto it = info.find(name);
    return it != info.end();
}

uniform_block_type::uniform_info uniform_block_type::get_info(
    const std::string& name
) const
{
    auto it = info.find(name);
    if(it == info.end())
    {
        throw std::runtime_error("No uniform named \""+name+"\" in block");
    }
    return it->second;
}

bool uniform_block_type::operator==(const uniform_block_type& other) const
{
    return size == other.size && info == other.info;
}

bool uniform_block_type::operator!=(const uniform_block_type& other) const
{
    return size != other.size || info != other.info;
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
