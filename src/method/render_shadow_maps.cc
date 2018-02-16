#include "render_shadow_maps.hh"

method::render_shadow_maps::render_shadow_maps(
    shader_store& store,
    render_scene* scene
): depth_shader(store.get(shader::path{"generic.vert", "depth.frag"}, {})),
   scene(scene)
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
    if(!depth_shader || !scene) return;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
}
