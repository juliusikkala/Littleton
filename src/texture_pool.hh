#ifndef TEXTURE_POOL_HH
#define TEXTURE_POOL_HH
#include "resource.hh"
#include "texture.hh"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class texture_pool: public virtual glresource
{
private:
    using map_type = std::unordered_map<
        std::string /* name */,
        std::unique_ptr<texture>
    >;

public:
    using iterator = map_type::iterator;
    using const_iterator = map_type::const_iterator;

    texture_pool(context& ctx);
    texture_pool(const texture_pool& other) = delete;
    texture_pool(texture_pool& other) = delete;
    ~texture_pool();

    texture* add_texture(const std::string& name, texture* tex);
    texture* add_texture(const std::string& name, texture&& tex);
    // Unsafe, deletes the pointer to the resource. Make sure there are no
    // references to it left. You are probably looking for texture::unload().
    void remove_texture(const std::string& name);
    const texture* get_texture(const std::string& name);

    bool contains_texture(const std::string& name);

    // Unloads all textures in this pool.
    void unload_all();

    const_iterator cbegin() const;
    const_iterator cend() const;

private:
    map_type textures;
};

#endif

