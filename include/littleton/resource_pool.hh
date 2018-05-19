/*
    Copyright 2018 Julius Ikkala

    This file is part of Littleton.

    Littleton is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Littleton is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Littleton.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef LT_RESOURCE_POOL_HH
#define LT_RESOURCE_POOL_HH
#include "api.hh"
#include "resource.hh"
#include "shader_pool.hh"
#include "framebuffer_pool.hh"
#include "texture.hh"
#include "gpu_buffer.hh"
#include "primitive.hh"
#include "sampler.hh"
#include "material.hh"
#include "model.hh"
#include <unordered_map>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <iterator>

namespace lt
{

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

    explicit generic_resource_pool(context& ctx);
    generic_resource_pool(const generic_resource_pool& other) = delete;
    generic_resource_pool(generic_resource_pool&& other) = delete;
    ~generic_resource_pool();

    T* add(const std::string& name, T* t, bool ignore_duplicate = false);
    T* add(const std::string& name, T&& t, bool ignore_duplicate = false);
    // Unsafe, deletes the pointer to the resource. Make sure there are no
    // references to it left. You are probably looking for resource::unload().
    void remove(const std::string& name);

    const T* get(const std::string& name) const;

    // Use this only if you are sure that modifying the resource globally is
    // fine. Typically, it isn't, but this may be useful if you need to modify
    // resources fresh from a loader.
    T* get_mutable(const std::string& name);

    bool contains(const std::string& name) const;

    const_iterator cbegin() const;
    const_iterator cend() const;

protected:
    map_type resources;
};

template<typename T>
class lazy_resource_pool: public generic_resource_pool<T>
{
public:
    explicit lazy_resource_pool(context& ctx);

    // Loads all resources in this pool.
    void load_all();

    // Unloads all resources in this pool.
    void unload_all();
};

using texture_pool = lazy_resource_pool<texture>;
using gpu_buffer_pool = lazy_resource_pool<gpu_buffer>;
using primitive_pool = lazy_resource_pool<primitive>;
using sampler_pool = generic_resource_pool<sampler>;
using material_pool = generic_resource_pool<material>;
using model_pool = generic_resource_pool<model>;

#define generic_resource_alias_decl(type) \
type* add_ ## type (\
    const std::string& name, type* t, bool ignore_duplicate = false); \
type* add_ ## type (\
    const std::string& name, type&& t, bool ignore_duplicate = false); \
void remove_ ## type(const std::string& name); \
const type* get_ ## type(const std::string& name) const; \
type* get_ ## type ## _mutable(const std::string& name); \
bool contains_ ## type(const std::string& name) const; \

class LT_API resource_pool
: public virtual glresource, public shader_pool, public texture_pool,
  public gpu_buffer_pool, public primitive_pool, public sampler_pool,
  public material_pool, public model_pool, public framebuffer_pool
{
public:
    resource_pool(
        context& ctx,
        const std::vector<std::string>& shader_path = {},
        const std::optional<std::string>& shader_binary_path = {}
    );
    resource_pool(const resource_pool& other) = delete;
    resource_pool(resource_pool&& other) = delete;
    ~resource_pool();

    framebuffer_pool::loaner loan_framebuffer(
        glm::uvec2 size,
        const framebuffer::target_specification_map& target_specifications,
        unsigned samples = 0
    );

    multishader* add_shader(const shader::path& path);
    void remove_shader(const shader::path& path);
    multishader* get_shader(const shader::path& path);
    shader* get_shader(
        const shader::path& path,
        const shader::definition_map& definitions
    );

    generic_resource_alias_decl(texture);
    generic_resource_alias_decl(gpu_buffer);
    generic_resource_alias_decl(primitive);
    generic_resource_alias_decl(sampler);
    generic_resource_alias_decl(material);
    generic_resource_alias_decl(model);

    void load_all();
    void unload_all();
};
#undef generic_resource_alias_decl
} // namespace lt

#include "resource_pool.tcc"
#endif
