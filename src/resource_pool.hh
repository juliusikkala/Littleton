#ifndef RESOURCE_POOL_HH
#define RESOURCE_POOL_HH
#include "resource.hh"
#include "shader_pool.hh"
#include "texture.hh"
#include "vertex_buffer.hh"
#include "sampler.hh"
#include "material.hh"
#include "model.hh"
#include <unordered_map>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <iterator>

template<typename T>
class generic_resource_pool: public virtual glresource
{
private:
    using map_type = std::unordered_map<
        std::string /* name */,
        std::unique_ptr<T>
    >;

public:
    using iterator = typename map_type::iterator;
    using const_iterator = typename map_type::const_iterator;

    generic_resource_pool(context& ctx);
    generic_resource_pool(const generic_resource_pool& other) = delete;
    generic_resource_pool(generic_resource_pool& other) = delete;
    ~generic_resource_pool();

    T* add(const std::string& name, T* t);
    T* add(const std::string& name, T&& t);
    // Unsafe, deletes the pointer to the resource. Make sure there are no
    // references to it left. You are probably looking for resource::unload().
    void remove(const std::string& name);
    const T* get(const std::string& name);

    bool contains(const std::string& name);

    // Unloads all textures in this pool.
    void unload_all();

    const_iterator cbegin() const;
    const_iterator cend() const;

private:
    map_type resources;
};

using texture_pool = generic_resource_pool<texture>;
using vertex_buffer_pool = generic_resource_pool<vertex_buffer>;
using sampler_pool = generic_resource_pool<sampler>;
using material_pool = generic_resource_pool<material>;
using model_pool = generic_resource_pool<model>;

#define generic_resource_alias_decl(type) \
type* add_ ## type (const std::string& name, type* t); \
type* add_ ## type (const std::string& name, type&& t); \
void remove_ ## type(const std::string& name); \
const type* get_ ## type(const std::string& name); \
bool contains_ ## type(const std::string& name); \

class resource_pool
: public virtual glresource, public shader_pool, public texture_pool,
  public vertex_buffer_pool, public sampler_pool, public material_pool,
  public model_pool
{
public:
    resource_pool(
        context& ctx,
        const std::vector<std::string>& shader_path = {},
        const std::optional<std::string>& shader_binary_path = {}
    );
    resource_pool(const resource_pool& other) = delete;
    resource_pool(resource_pool& other) = delete;
    ~resource_pool();

    multishader* add_shader(const shader::path& path);
    void remove_shader(const shader::path& path);
    multishader* get_shader(const shader::path& path);
    shader* get_shader(
        const shader::path& path,
        const shader::definition_map& definitions
    );

    generic_resource_alias_decl(texture);
    generic_resource_alias_decl(vertex_buffer);
    generic_resource_alias_decl(sampler);
    generic_resource_alias_decl(material);
    generic_resource_alias_decl(model);

    void unload_all();
};
#undef generic_resource_alias_decl

#include "resource_pool.tcc"
#endif
