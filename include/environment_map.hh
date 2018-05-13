#ifndef LT_ENVIRONMENT_MAP_HH
#define LT_ENVIRONMENT_MAP_HH
#include "glheaders.hh"
#include "texture.hh"
#include "math.hh"
#include <string>

namespace lt
{

class environment_map: public texture
{
public:
    environment_map(
        context& ctx,
        const std::string& path,
        bool srgb = false
    );

    environment_map(
        context& ctx,
        glm::uvec2 size,
        GLint format,
        GLenum type,
        const void* data = nullptr
    );

    environment_map(const environment_map& other) = delete;
    environment_map(environment_map&& other);
    ~environment_map();

    static environment_map* create(
        context& ctx,
        const std::string& path,
        bool srgb = false
    );

    static environment_map* create(
        context& ctx,
        glm::uvec2 size,
        GLint internal_format,
        GLenum type,
        size_t data_size = 0,
        const void* data = nullptr
    );
protected:
    environment_map(context& ctx);
};

} // namespace lt

#endif
