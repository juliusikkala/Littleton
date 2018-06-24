#include <cstring>

namespace lt
{

template<typename T>
bool uniform_is_compatible(GLenum type, GLint size, size_t count)
{
    // The graphics drivers are allowed to shrink arrays, so allow count to be
    // >= size.
    if((unsigned)size > count) return false;

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
        case GL_IMAGE_1D:
        case GL_IMAGE_2D:
        case GL_IMAGE_3D:
        case GL_IMAGE_2D_RECT:
        case GL_IMAGE_CUBE:
        case GL_IMAGE_BUFFER:
        case GL_IMAGE_1D_ARRAY:
        case GL_IMAGE_2D_ARRAY:
        case GL_IMAGE_CUBE_MAP_ARRAY:
        case GL_IMAGE_2D_MULTISAMPLE:
        case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
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

template<typename T>
bool uniform_block_type::is_compatible(const std::string& name, size_t count)
{
    auto it = uniforms.find(name);
    if(it == uniforms.end()) return false;

    uniform_info& info = it->second;

    return uniform_is_compatible<T>(info.type, info.size, count);
}

template<typename T>
void uniform_block::set(const std::string& name, const T& value)
{
    set<T>(name, 1, &value);
}

template<typename T>
void uniform_block::set(const std::string& name, size_t count, const T* value)
{
    auto it = type.uniforms.find(name);
    if(it == type.uniforms.end())
        throw std::runtime_error("No uniform named " + name);

    uniform_block_type::uniform_info& info = it->second;

    if(!uniform_is_compatible<T>(info.type, info.size, count))
        throw std::runtime_error("Wrong type for "+name);

    GLint offset = info.offset;
    for(GLint i = 0; i < info.size; ++i)
    {
        // Use matrix_stride if the uniform is a matrix
        if constexpr(
            std::is_same_v<T, glm::mat2> ||
            std::is_same_v<T, glm::mat3> ||
            std::is_same_v<T, glm::mat4>
        ) {
            GLint mat_offset = offset;
            for(GLint j = 0; j < value[i].length(); ++j)
            {
                auto& vec = value[i][j];
                memcpy(buffer + mat_offset, &vec, sizeof(vec));
                mat_offset += info.matrix_stride;
            }
        }
        else memcpy(buffer + offset, value + i, sizeof(T));

        offset += info.array_stride;
    }
}

} // namespace lt
