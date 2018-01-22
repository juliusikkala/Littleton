#include <glm/glm.hpp>
#include <stdexcept>

template<typename T>
void uniform_set_value(GLint location, size_t count, const T* value);

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

template<typename T>
bool shader::is_compatible(const std::string& name, size_t count)
{
    return is_compatible<T>(uniforms.find(name), count);
}

template<typename T>
void shader::set(const std::string& name, const T& value)
{
    auto it = uniforms.find(name);
    if(it == uniforms.end()) return;

    uniform_data& data = it->second;

    if(!is_compatible<T>(data.type, data.size))
        throw std::runtime_error("Wrong type for "+name);

    bind();
    uniform_set_value<T>(data.location, 1, &value);
}

template<typename T>
void shader::set(
    const std::string& name,
    size_t count,
    const T* value
){
    auto it = uniforms.find(name);
    if(it == uniforms.end()) return;

    uniform_data& data = it->second;

    if(!is_compatible<T>(data.type, data.size, count))
        throw std::runtime_error("Wrong type for "+name);

    bind();
    uniform_set_value<T>(data.location, count, value);
}

template<typename T>
bool shader::is_compatible(GLenum type, GLint size, size_t count)
{
    if(size != count) return false;

    if constexpr (std::is_same_v<T, float>)
        return type == GL_FLOAT;
    if constexpr (std::is_same_v<T, glm::vec2>)
        return type == GL_FLOAT_VEC2;
    if constexpr (std::is_same_v<T, glm::vec3>)
        return type == GL_FLOAT_VEC3;
    if constexpr (std::is_same_v<T, glm::vec4>)
        return type == GL_FLOAT_VEC4;

    if constexpr (std::is_same_v<T, int>)
        switch(type)
        {
        case GL_INT:
        case GL_BOOL:
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_BUFFER:
        case GL_SAMPLER_2D_RECT:
        case GL_SAMPLER_2D_RECT_SHADOW:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_2D_RECT:
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
            return true;
        default:
            return false;
        }
    if constexpr (std::is_same_v<T, glm::ivec2>)
        return type == GL_INT_VEC2 || type == GL_BOOL_VEC2;
    if constexpr (std::is_same_v<T, glm::ivec3>)
        return type == GL_INT_VEC3 || type == GL_BOOL_VEC2;
    if constexpr (std::is_same_v<T, glm::ivec4>)
        return type == GL_INT_VEC4 || type == GL_BOOL_VEC4;

    if constexpr (std::is_same_v<T, unsigned>)
        return type == GL_UNSIGNED_INT;
    if constexpr (std::is_same_v<T, glm::uvec2>)
        return type == GL_UNSIGNED_INT_VEC2;
    if constexpr (std::is_same_v<T, glm::uvec3>)
        return type == GL_UNSIGNED_INT_VEC3;
    if constexpr (std::is_same_v<T, glm::uvec4>)
        return type == GL_UNSIGNED_INT_VEC4;

    if constexpr (std::is_same_v<T, bool>)
        return type == GL_BOOL;

    if constexpr (std::is_same_v<T, glm::mat2>)
        return type == GL_FLOAT_MAT2;
    if constexpr (std::is_same_v<T, glm::mat3>)
        return type == GL_FLOAT_MAT3;
    if constexpr (std::is_same_v<T, glm::mat4>)
        return type == GL_FLOAT_MAT4;

    return false;
}

