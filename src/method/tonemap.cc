#include "tonemap.hh"
#include "render_target.hh"
#include "texture.hh"
#include "resource_pool.hh"
#include "common_resources.hh"

method::tonemap::tonemap(
    render_target& target,
    texture& src,
    resource_pool& pool,
    float exposure
):  target_method(target), src(&src),
    tonemap_shader(
        pool.get_shader(shader::path{"fullscreen.vert", "tonemap.frag"}, {})
    ),
    quad(common::ensure_quad_vertex_buffer(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool)),
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
    tonemap_shader->set("in_color", fb_sampler.bind(*src));

    quad.draw();
}

std::string method::tonemap::get_name() const
{
    return "tonemap";
}
