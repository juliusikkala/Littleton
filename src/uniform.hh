#ifndef UNIFORM_HH
#define UNIFORM_HH
#include "glheaders.hh"
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>
#include <memory>

template<typename T>
bool uniform_is_compatible(GLenum type, GLint size, size_t count = 1);

template<typename T>
void uniform_set_value(GLint location, size_t count, const T* value);

template<> void uniform_set_value<float>(
    GLint location, size_t count, const float* value);
template<> void uniform_set_value<glm::vec2>(
    GLint location, size_t count, const glm::vec2* value);
template<> void uniform_set_value<glm::vec3>(
    GLint location, size_t count, const glm::vec3* value);
template<> void uniform_set_value<glm::vec4>(
    GLint location, size_t count, const glm::vec4* value);
template<> void uniform_set_value<int>(
    GLint location, size_t count, const int* value);
template<> void uniform_set_value<glm::ivec2>(
    GLint location, size_t count, const glm::ivec2* value);
template<> void uniform_set_value<glm::ivec3>(
    GLint location, size_t count, const glm::ivec3* value);
template<> void uniform_set_value<glm::ivec4>(
    GLint location, size_t count, const glm::ivec4* value);
template<> void uniform_set_value<unsigned>(
    GLint location, size_t count, const unsigned* value);
template<> void uniform_set_value<glm::uvec2>(
    GLint location, size_t count, const glm::uvec2* value);
template<> void uniform_set_value<glm::uvec3>(
    GLint location, size_t count, const glm::uvec3* value);
template<> void uniform_set_value<glm::uvec4>(
    GLint location, size_t count, const glm::uvec4* value);
template<> void uniform_set_value<bool>(
    GLint location, size_t count, const bool* value);
template<> void uniform_set_value<glm::mat2>(
    GLint location, size_t count, const glm::mat2* value);
template<> void uniform_set_value<glm::mat3>(
    GLint location, size_t count, const glm::mat3* value);
template<> void uniform_set_value<glm::mat4>(
    GLint location, size_t count, const glm::mat4* value);


class uniform_block_type
{
friend class uniform_block;
public:
    struct uniform_info
    {
        GLint offset;
        GLint size;
        GLint array_stride;
        GLint matrix_stride;
        GLenum type;

        bool operator==(const uniform_info& other) const;
        bool operator!=(const uniform_info& other) const;
    };

    uniform_block_type(const uniform_block_type& other);
    uniform_block_type(uniform_block_type&& other);

    // Use this only when you know what you are doing.
    uniform_block_type(
        std::unordered_map<std::string, uniform_info>&& uniforms,
        size_t size
    );

    size_t get_total_size() const;
    bool exists(const std::string& name) const;
    uniform_info get_info(const std::string& name) const;

    template<typename T>
    bool is_compatible(const std::string& name, size_t count = 1);

    bool operator==(const uniform_block_type& other) const;
    bool operator!=(const uniform_block_type& other) const;

private:
    std::unordered_map<std::string, uniform_info> uniforms;

    size_t size;
};

class uniform_block
{
public:
    uniform_block(const uniform_block_type& type);
    uniform_block(const uniform_block& block);
    uniform_block(uniform_block&& block);
    ~uniform_block();

    const uniform_block_type& get_type() const;

    template<typename T>
    void set(const std::string& name, const T& value);

    template<typename T>
    void set(const std::string& name, size_t count, const T* value);

    void bind(unsigned index = 0) const;

    // Upload the block once you have set all the variables you are going to
    // set. Generally, you'll want to upload before binding.
    void upload();

private:
    void basic_load();
    void basic_unload();

    uniform_block_type type;
    GLuint ubo;
    uint8_t* buffer;
};

#include "uniform.tcc"

#endif
