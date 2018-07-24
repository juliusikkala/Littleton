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
#include "scene_graph.hh"
#include "scene.hh"
#include <stdexcept>

namespace lt
{

scene_graph::scene_graph() { }

scene_graph::scene_graph(scene_graph&& other)
: objects(std::move(other.objects))
{
}

object* scene_graph::add_object(
    const std::string& name,
    object&& obj,
    bool ignore_if_exists
){
    auto it = objects.find(name);
    if(it != objects.end())
    {
        if(ignore_if_exists)
            return &it->second;
        else
            throw std::runtime_error("Object " + name + " already exists");
    }
    auto pair = objects.emplace(name, std::move(obj));
    return &pair.first->second;
}

bool scene_graph::has_object(const std::string& name)
{
    return objects.count(name);
}

object* scene_graph::get_object(const std::string& name)
{
    auto it = objects.find(name);
    if(it == objects.end())
        throw std::runtime_error("Unable to find object " + name);
    return &it->second;
}

const object* scene_graph::get_object(const std::string& name) const
{
    auto it = objects.find(name);
    if(it == objects.end())
        throw std::runtime_error("Unable to find object " + name);
    return &it->second;
}

void scene_graph::remove_object(const std::string& name)
{
    objects.erase(name);
}

void scene_graph::add_to_scene(object_scene* scene)
{
    for(auto& pair: objects)
    {
        scene->add_object(&pair.second);
    }
}

void scene_graph::merge(const scene_graph& other)
{
    std::map<
        const transformable_node*,
        transformable_node*
    > parent_transfer;

    // Clone objects
    for(auto& pair: other.objects)
    {
        auto it = objects.insert(pair);
        parent_transfer.insert({&pair.second, &it.first->second});
    }

    // Fix parents
    for(auto& pair: objects)
    {
        auto it = parent_transfer.find(pair.second.get_parent());
        if(it != parent_transfer.end())
        {
            pair.second.set_parent(it->second);
        }
    }
}

} // namespace lt

