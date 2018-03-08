#include "tonemap.hh"
#include "render_target.hh"
#include "texture.hh"
#include "shader_pool.hh"

method::tonemap::tonemap(
    render_target& target,
    texture& src,
    shader_pool& pool,
    float exposure
):  target_method(target), src(&src),
    tonemap_shader(
        pool.get_shader(shader::path{"fullscreen.vert", "tonemap.frag"}, {})
    ),
    fullscreen_quad(vertex_buffer::create_square(target.get_context())),
    color_sampler(
        target.get_context(),
        GL_NEAREST,
        GL_NEAREST,
        GL_CLAMP_TO_EDGE
    ),
    exposure(exposure)
{
}

void method::tonemap::set_exposure(float exposure)
{
    this->exposure = exposure;
}

float method::tonemap::get_exposure() const
{
    return exposure;
}

void method::tonemap::execute()
{
    target_method::execute();

    if(!tonemap_shader || !src) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    tonemap_shader->bind();
    tonemap_shader->set<float>("exposure", exposure);
    tonemap_shader->set("in_color", color_sampler.bind(src->bind()));

    fullscreen_quad.draw();
}

std::string method::tonemap::get_name() const
{
    return "tonemap";
}
