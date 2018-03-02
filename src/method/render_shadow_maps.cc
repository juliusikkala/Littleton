#include "render_shadow_maps.hh"
#include "shadow/shadow_map.hh"

method::render_shadow_maps::render_shadow_maps(
    shader_store& store,
    render_scene* scene
): store(&store), scene(scene)
{}

void method::render_shadow_maps::set_scene(render_scene* s)
{
    scene = s;
}

render_scene* method::render_shadow_maps::get_scene() const
{
    return scene;
}

void method::render_shadow_maps::execute()
{
    if(!store || !scene) return;

    for(auto& pair: scene->get_shadow_maps())
    {
        for(basic_shadow_map* shadow_map: pair.second)
        {
            shadow_map->render(*store, scene, pair.first.get());
        }
    }
}
