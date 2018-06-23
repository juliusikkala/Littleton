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
#include "multishader.hh"
#include "helpers.hh"
#include "shader.hh"
#include <boost/filesystem.hpp>

namespace lt
{

multishader::multishader(
    context& ctx,
    const shader::source& source,
    const std::vector<std::string>& include_path,
    const std::optional<std::string>& shader_binary_path
): glresource(ctx), source(source), include_path(include_path),
   shader_binary_path(shader_binary_path)
{}

multishader::multishader(
    context& ctx,
    const shader::path& path,
    const std::vector<std::string>& include_path,
    const std::optional<std::string>& shader_binary_path
): glresource(ctx), source(path), include_path(include_path),
   shader_binary_path(shader_binary_path)
{
    this->include_path.push_back(
        boost::filesystem::path(path.vert).parent_path().string()
    );
    this->include_path.push_back(
        boost::filesystem::path(path.frag).parent_path().string()
    );
    this->include_path.push_back(
        boost::filesystem::path(path.geom).parent_path().string()
    );
    this->include_path.push_back(
        boost::filesystem::path(path.comp).parent_path().string()
    );
}

multishader::multishader(multishader&& other)
: glresource(other.get_context()),
  source(std::move(other.source)),
  include_path(std::move(other.include_path)),
  cache(std::move(other.cache))
{}

void multishader::clear()
{
    cache.clear();
}

void multishader::unload()
{
    for(auto& pair: cache) pair.second->unload();
}

shader* multishader::get(const shader::definition_map& definitions) const
{
    auto it = cache.find(definitions);
    // Cache miss
    if(it == cache.end())
    {
        if(shader_binary_path)
        {
            boost::filesystem::path path(append_hash_to_path(
                shader_binary_path.value(),
                definitions,
                ".bin"
            ));

            shader* s = shader::create(
                get_context(),
                source,
                definitions,
                include_path,
                path.string()
            );

            cache[definitions].reset(s);

            return s;
        }
        else
        {
            shader* s = shader::create(
                get_context(),
                source,
                definitions,
                include_path
            );
            cache[definitions].reset(s);
            return s;
        }
    }
    return it->second.get();
}

} // namespace lt
