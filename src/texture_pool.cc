#include "texture_pool.hh"
#include <stdexcept>

texture_pool::texture_pool(context& ctx)
: glresource(ctx) { }
texture_pool::~texture_pool() { }

texture* texture_pool::add_texture(const std::string& name, texture* tex)
{
    textures.emplace(name, tex);
    return tex;
}

texture* texture_pool::add_texture(const std::string& name, texture&& tex)
{
    texture* new_tex = new texture(std::move(tex));
    textures.emplace(name, new_tex);
    return new_tex;
}

void texture_pool::remove_texture(const std::string& name)
{
    textures.erase(name);
}

const texture* texture_pool::get_texture(const std::string& name)
{
    auto it = textures.find(name);
    if(it == textures.end())
        throw std::runtime_error("Unable to get texture " + name);
    return it->second.get();
}

bool texture_pool::contains_texture(const std::string& name)
{
    auto it = textures.find(name);
    return it != textures.end();
}

void texture_pool::unload_all()
{
    for(auto& pair: textures)
    {
        pair.second->unload();
    }
}

texture_pool::const_iterator texture_pool::cbegin() const
{
    return textures.cbegin();
}

texture_pool::const_iterator texture_pool::cend() const
{
    return textures.cend();
}
