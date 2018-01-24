#include "fullscreen_effect.hh"
#include "helpers.hh"

method::fullscreen_effect::fullscreen_effect(shader* effect)
: effect(effect), fullscreen_quad(vertex_buffer::create_fullscreen()) {}

method::fullscreen_effect::~fullscreen_effect() { }

void method::fullscreen_effect::execute()
{
    if(!effect) return;

    glDisable(GL_DEPTH_TEST);
    effect->bind();
    fullscreen_quad.draw();
}

void method::fullscreen_effect::set_shader(shader* effect)
{
    this->effect = effect;
}

shader* method::fullscreen_effect::get_shader() const
{
    return effect;
}
