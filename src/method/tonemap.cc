#include "tonemap.hh"

method::tonemap::tonemap(
    render_target& target,
    texture& src,
    shader_store& store,
    float exposure
): target_method(target), src(&src),
   tonemap_shader(
        store.get(shader::path{"fullscreen.vert", "tonemap.frag"}, {})
   ),
   fullscreen_quad(vertex_buffer::create_square(target.get_context())),
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
    tonemap_shader->set("in_color", src->bind());

    fullscreen_quad.draw();
}
