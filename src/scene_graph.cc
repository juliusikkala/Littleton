#include "scene_graph.hh"
#include "scene.hh"
#include <stdexcept>

scene_graph::scene_graph() { }

scene_graph::scene_graph(scene_graph&& other)
: objects(std::move(other.objects))
{
}

object* scene_graph::add_object(const std::string& name, object&& obj)
{
    auto it = objects.find(name);
    if(it != objects.end())
        throw std::runtime_error("Object " + name + " already exists");
    auto pair = objects.emplace(name, std::move(obj));
    return &pair.first->second;
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

void scene_graph::add_to_scene(render_scene* scene)
{
    for(auto& pair: objects)
    {
        scene->add_object(&pair.second);
    }
}

