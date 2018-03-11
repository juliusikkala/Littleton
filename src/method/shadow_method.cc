#include "shadow_method.hh"

method::shadow_method::shadow_method(render_scene* scene)
: scene(scene)
{
}

void method::shadow_method::set_scene(render_scene* scene)
{
    this->scene = scene;
}

render_scene* method::shadow_method::get_scene() const
{
    return scene;
}

void method::shadow_method::set_directional_uniforms(shader*, unsigned&) {}
