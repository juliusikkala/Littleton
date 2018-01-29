#include <glm/glm.hpp>
#include <stdexcept>

template<typename T>
bool shader::is_compatible(const std::string& name, size_t count) const
{
    load();
    auto it = uniforms.find(name);
    if(it == uniforms.end()) return true;

    uniform_data& data = it->second;

    return uniform_is_compatible<T>(data.type, data.size, count);
}

template<typename T>
void shader::set(const std::string& name, const T& value)
{
    load();
    auto it = uniforms.find(name);
    if(it == uniforms.end()) return;

    uniform_data& data = it->second;

    if(!uniform_is_compatible<T>(data.type, data.size))
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
    load();
    auto it = uniforms.find(name);
    if(it == uniforms.end()) return;

    uniform_data& data = it->second;

    if(!uniform_is_compatible<T>(data.type, data.size, count))
        throw std::runtime_error("Wrong type for "+name);

    bind();
    uniform_set_value<T>(data.location, data.size, value);
}

