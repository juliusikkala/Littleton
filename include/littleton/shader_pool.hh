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
#ifndef LT_SHADER_POOL_HH
#define LT_SHADER_POOL_HH
#include "api.hh"
#include "resource.hh"
#include "shader.hh"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <boost/functional/hash.hpp>

namespace lt
{

class multishader;
class LT_API shader_pool: public virtual glresource
{
private:
    using map_type = std::unordered_map<
        shader::path,
        std::unique_ptr<multishader>,
        boost::hash<shader::path>
    >;

public:
    using iterator = map_type::iterator;
    using const_iterator = map_type::const_iterator;

    shader_pool(
        context& ctx,
        const std::vector<std::string>& shader_path = {},
        const std::optional<std::string>& shader_binary_path = {}
    );
    shader_pool(const shader_pool& other) = delete;
    shader_pool(shader_pool&& other) = delete;
    ~shader_pool();

    multishader* add(const shader::path& path);

    // Unsafe, deletes the pointer to the resource. Make sure there are no
    // references to it left. You are probably looking for shader::unload().
    void remove(const shader::path& path);

    // This deletes all binaries in shader_binary_path
    void delete_binaries();

    // Unloads all shaders in this pool.
    void unload_all();

    multishader* get(const shader::path& path);
    shader* get(
        const shader::path& path,
        const shader::definition_map& definitions
    );

    iterator begin();
    iterator end();

    const_iterator cbegin() const;
    const_iterator cend() const;

private:
    std::vector<std::string> shader_path;
    std::optional<std::string> shader_binary_path;
    map_type shaders;
};

} // namespace lt

#endif

