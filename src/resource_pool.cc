#include "resource_pool.hh"

legacy_resource_pool::container::~container() {};

legacy_resource_pool::legacy_resource_pool(context& ctx): glresource(ctx) { }
legacy_resource_pool::~legacy_resource_pool() { }

resource_pool::resource_pool(
    context& ctx,
    const std::vector<std::string>& shader_path,
    const std::optional<std::string>& shader_binary_path
):  glresource(ctx), legacy_resource_pool(ctx),
    shader_pool(ctx, shader_path, shader_binary_path),
    texture_pool(ctx)
{
}
resource_pool::~resource_pool() {}

multishader* resource_pool::add_shader(const shader::path& path)
{
    return shader_pool::add(path);
}

void resource_pool::remove_shader(const shader::path& path)
{
    shader_pool::remove(path);
}

multishader* resource_pool::get_shader(const shader::path& path)
{
    return shader_pool::get(path);
}

shader* resource_pool::get_shader(
    const shader::path& path,
    const shader::definition_map& definitions
){
    return shader_pool::get(path, definitions);
}

texture* resource_pool::add_texture(const std::string& name, texture* t)
{
    return texture_pool::add(name, t);
}

texture* resource_pool::add_texture(const std::string& name, texture&& t)
{
    return texture_pool::add(name, std::move(t));
}

void resource_pool::remove_texture(const std::string& name)
{
    texture_pool::remove(name);
}

const texture* resource_pool::get_texture(const std::string& name)
{
    return texture_pool::get(name);
}

bool resource_pool::contains_texture(const std::string& name)
{
    return texture_pool::contains(name);
}

void resource_pool::unload_all()
{
    shader_pool::unload_all();
    texture_pool::unload_all();
}

