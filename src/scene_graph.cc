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

void scene_graph::add_to_scene(render_scene* scene)
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

