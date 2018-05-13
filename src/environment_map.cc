#include "environment_map.hh"
#include <cstring>

namespace lt
{

environment_map::environment_map(context& ctx)
: texture(ctx)
{
}

environment_map::environment_map(
    context& ctx,
    const std::string& path,
    bool srgb
): texture(ctx, path, srgb, GL_TEXTURE_CUBE_MAP)
{
}

environment_map::environment_map(
    context& ctx,
    glm::uvec2 size,
    GLint format,
    GLenum type,
    const void* data
): texture(ctx, size, format, type, 0, GL_TEXTURE_CUBE_MAP, data)
{
}

environment_map::environment_map(environment_map&& other)
: texture(std::move(other))
{
}

environment_map::~environment_map() {}

class file_environment_map: public environment_map
{
public:
    file_environment_map(
        context& ctx,
        const std::string& path,
        bool srgb
    ): environment_map(ctx), srgb(srgb), path(path) {}

protected:
    void load_impl() const override
    {
        basic_load(path, srgb, GL_TEXTURE_CUBE_MAP);
    }

    void unload_impl() const override
    {
        basic_unload();
    }

private:
    bool srgb;
    std::string path;
};

environment_map* environment_map::create(
    context& ctx,
    const std::string& path,
    bool srgb
){
    return new file_environment_map(ctx, path, srgb);
}

class data_environment_map: public environment_map
{
public:
    data_environment_map(
        context& ctx,
        glm::uvec2 size,
        GLint internal_format,
        GLenum type,
        size_t data_size,
        const void* data
    ): environment_map(ctx), size(size)
    {
        this->internal_format = internal_format;
        this->type = type;

        if(data)
        {
            this->data = new uint8_t[data_size];
            memcpy(this->data, data, data_size);
        }
    }

    ~data_environment_map()
    {
        if(data) delete [] (uint8_t*)data;
    }

protected:
    void load_impl() const override
    {
        basic_load(
            size,
            internal_format,
            type,
            0,
            GL_TEXTURE_CUBE_MAP,
            data
        );
    }

    void unload_impl() const override
    {
        basic_unload();
    }

private:
    glm::uvec2 size;
    void* data;
};

environment_map* environment_map::create(
    context& ctx,
    glm::uvec2 size,
    GLint internal_format,
    GLenum type,
    size_t data_size,
    const void* data
){
    return new data_environment_map(
        ctx, size, internal_format, type, data_size, data
    );
}

} // namespace lt
