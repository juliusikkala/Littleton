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
#ifndef LT_LOADERS_HH
#define LT_LOADERS_HH
#include "api.hh"
#include <string>
#include <unordered_map>

namespace lt
{

class resource_pool;
class scene_graph;

// GLTF files may have several scenes; return them by name.
LT_API std::unordered_map<std::string, scene_graph> load_gltf(
    resource_pool& pool,
    const std::string& path,
    const std::string& data_prefix = "",
    bool ignore_duplicates = true
);

} // namespace lt

#endif
