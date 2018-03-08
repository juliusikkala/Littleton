#ifndef SCENE_GRAPH_HH
#define SCENE_GRAPH_HH
#include "object.hh"
#include <unordered_map>
#include <string>

class render_scene;
// The naming might be a bit off, this scene_graph simply acts as a container
// for nodes that can be referenced from a scene.
// TODO: Add more node types besides object
class scene_graph
{
public:
    scene_graph();
    scene_graph(scene_graph&& other);

    object* add_object(const std::string& name, object&& obj);
    object* get_object(const std::string& name);
    const object* get_object(const std::string& name) const;
    void remove_object(const std::string& name);

    void add_to_scene(render_scene* scene);

private:
    std::unordered_map<std::string, object> objects;
};
#endif
