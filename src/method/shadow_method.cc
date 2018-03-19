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
void method::shadow_method::set_omni_uniforms(shader*, unsigned&) {}
void method::shadow_method::set_perspective_uniforms(shader*, unsigned&) {}

shader::definition_map
method::shadow_method::get_directional_definitions() const
{
    return {};
}

shader::definition_map method::shadow_method::get_omni_definitions() const
{
    return {};
}

shader::definition_map
method::shadow_method::get_perspective_definitions() const
{
    return {};
}

void method::shadow_method::set_shadow_map_uniforms(
    shader*,
    unsigned&,
    directional_shadow_map*,
    const std::string&,
    const glm::mat4& 
){}

void method::shadow_method::set_shadow_map_uniforms(
    shader*,
    unsigned&,
    omni_shadow_map*,
    const std::string&,
    const glm::mat4&
){}

void method::shadow_method::set_shadow_map_uniforms(
    shader*,
    unsigned&,
    perspective_shadow_map*,
    const std::string&,
    const glm::mat4&
){}
