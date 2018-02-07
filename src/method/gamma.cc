#include "gamma.hh"

method::gamma::gamma(
    render_target& target,
    texture& src,
    shader_store& store,
    float g
): pipeline_method(target), src(&src), g(g),
   gamma_shader(store.get(shader::path{"fullscreen.vert", "gamma.frag"}, {})),
   fullscreen_quad(vertex_buffer::create_square(target.get_context()))
{
}

void method::gamma::set_gamma(float g)
{
    this->g = g;
}

float method::gamma::get_gamma() const
{
    return g;
}

void method::gamma::execute()
{
    if(!gamma_shader || !src) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    gamma_shader->bind();
    gamma_shader->set("gamma", 1.0f/g);
    gamma_shader->set("in_color", src->bind());

    fullscreen_quad.draw();
}