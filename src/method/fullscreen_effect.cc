#include "fullscreen_effect.hh"
#include "helpers.hh"
#include "render_target.hh"
#include "shader.hh"
#include "texture.hh"
#include "common_resources.hh"

namespace lt::method
{

fullscreen_effect::fullscreen_effect(
    render_target& target,
    resource_pool& pool,
    shader* effect
): target_method(target), effect(effect),
   quad(common::ensure_quad_primitive(pool))
{}

fullscreen_effect::~fullscreen_effect() { }

void fullscreen_effect::execute()
{
    target_method::execute();

    if(!effect) return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    effect->bind();

    quad.draw();
}

void fullscreen_effect::set_shader(shader* effect)
{
    this->effect = effect;
}

shader* fullscreen_effect::get_shader() const
{
    return effect;
}

std::string fullscreen_effect::get_name() const
{
    return "fullscreen_effect";
}

} // namespace lt::method
