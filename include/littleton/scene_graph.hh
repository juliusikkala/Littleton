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
#ifndef LT_SCENE_GRAPH_HH
#define LT_SCENE_GRAPH_HH
#include "../api.hh"
#include "object.hh"
#include <unordered_map>
#include <string>

namespace lt
{

class render_scene;
// The naming might be a bit off, this scene_graph simply acts as a container
// for nodes that can be referenced from a scene.
// TODO: Add more node types besides object
class LT_API scene_graph
{
public:
    scene_graph();
    scene_graph(scene_graph&& other);

    object* add_object(
        const std::string& name,
        object&& obj,
        bool ignore_if_exists = false
    );
    bool has_object(const std::string& name);

    object* get_object(const std::string& name);
    const object* get_object(const std::string& name) const;
    void remove_object(const std::string& name);

    void add_to_scene(render_scene* scene);

    void merge(const scene_graph& other);

private:
    std::unordered_map<std::string, object> objects;
};

} // namespace lt

#endif
