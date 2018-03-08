#include "render_shadow_maps.hh"
#include "shader_pool.hh"
#include "scene.hh"
#include "shadow/shadow_map.hh"

method::render_shadow_maps::render_shadow_maps(
    shader_pool& pool,
    render_scene* scene
): pool(&pool), scene(scene)
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
    if(!pool || !scene) return;

    for(auto& pair: scene->get_shadow_maps())
    {
        pair.first->render(*pool, pair.second, scene);
    }
}

std::string method::render_shadow_maps::get_name() const
{
    return "render_shadow_maps";
}
