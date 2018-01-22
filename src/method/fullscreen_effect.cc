#include "fullscreen_effect.hh"
#include "helpers.hh"

method::fullscreen_effect::fullscreen_effect(const std::string& fshader_source)
: effect(read_text_file("data/shaders/fullscreen.vert"), fshader_source),
  fullscreen_quad(vertex_buffer::create_fullscreen())
{}

method::fullscreen_effect::~fullscreen_effect() { }

void method::fullscreen_effect::execute()
{
    effect.bind();
    fullscreen_quad.draw();
}

shader* method::fullscreen_effect::get_shader()
{
    return &effect;
}

const shader* method::fullscreen_effect::get_shader() const
{
    return &effect;
}
