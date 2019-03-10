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
#include "shader_pool.hh"
#include "multishader.hh"
#include "helpers.hh"
#include <boost/filesystem.hpp>

namespace
{

std::string find_file(
    const std::vector<std::string> prefixes,
    const std::string& suffix
){
    boost::filesystem::path suffix_path(suffix);
    for(std::string prefix: prefixes)
    {
        boost::filesystem::path prefix_path(prefix);
        boost::filesystem::path combined(prefix_path/suffix_path);
        
        if(boost::filesystem::exists(combined))
            return combined.string();
    }
    throw std::runtime_error("Unable to find shader source " + suffix);
}

}

namespace lt
{
shader_pool::shader_pool(
    context& ctx,
    const std::vector<std::string>& shader_path,
    const std::optional<std::string>& shader_binary_path
):  glresource(ctx), parent(nullptr), shader_path(shader_path),
    shader_binary_path(shader_binary_path)
{}

shader_pool::shader_pool(
    shader_pool* parent,
    const std::vector<std::string>& shader_path,
    const std::optional<std::string>& shader_binary_path
):  glresource(parent->get_context()), parent(parent), shader_path(shader_path),
    shader_binary_path(shader_binary_path)
{}

shader_pool::~shader_pool()
{
}

multishader* shader_pool::add(const shader::path& path)
{
    auto it = shaders.find(path);
    if(it != shaders.end())
        throw std::runtime_error(
            "Shader with source files [" + path.vert + ", " + path.frag + ", " +
            path.geom + ", " + path.comp + "] already exists!"
        );

    shader::path full_path{
        path.vert.empty() ? "" : find_file(shader_path, path.vert),
        path.frag.empty() ? "" : find_file(shader_path, path.frag),
        path.geom.empty() ? "" : find_file(shader_path, path.geom),
        path.comp.empty() ? "" : find_file(shader_path, path.comp)
    };

    multishader* s;
    if(shader_binary_path)
    {
        s = new multishader(
            get_context(),
            full_path,
            shader_path,
            append_hash_to_path(shader_binary_path.value(), full_path)
        ); 
    }
    else s = new multishader(get_context(), full_path, shader_path); 

    shaders[path] = std::unique_ptr<multishader>(s);
    return s;
}

void shader_pool::remove(const shader::path& path)
{
    shaders.erase(path);
}

void shader_pool::delete_binaries()
{
    if(shader_binary_path)
    {
        boost::filesystem::remove_all(shader_binary_path.value());
    }
}

void shader_pool::unload_all()
{
    for(auto& pair: shaders)
    {
        pair.second->unload();
    }
}

multishader* shader_pool::get(const shader::path& path)
{
    multishader* ms = try_get(path);
    if(ms) return ms;
    return add(path);
}

multishader* shader_pool::try_get(const shader::path& path)
{
    auto it = shaders.find(path);
    if(it == shaders.end())
    {
        if(parent) return parent->try_get(path);
        return nullptr;
    }
    return it->second.get();
}

shader* shader_pool::get(
    const shader::path& path,
    const shader::definition_map& definitions
){
    return get(path)->get(definitions);
}

shader_pool::iterator shader_pool::begin() { return shaders.begin(); }
shader_pool::iterator shader_pool::end() { return shaders.end(); }

shader_pool::const_iterator shader_pool::cbegin() const
{
    return shaders.cbegin(); 
}

shader_pool::const_iterator shader_pool::cend() const
{
    return shaders.cend();
}

} // namespace lt
