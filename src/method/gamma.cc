#include "gamma.hh"
#include "render_target.hh"
#include "texture.hh"
#include "shader_store.hh"

method::gamma::gamma(
    render_target& target,
    texture& src,
    shader_store& store,
    float g
): target_method(target), src(&src), g(g),
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
    target_method::execute();

    if(!gamma_shader || !src) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    gamma_shader->bind();
    gamma_shader->set("gamma", 1.0f/g);
    gamma_shader->set("in_color", src->bind());

    fullscreen_quad.draw();
}

std::string method::gamma::get_name() const
{
    return "gamma";
}
