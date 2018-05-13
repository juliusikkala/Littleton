#include "gamma.hh"
#include "render_target.hh"
#include "texture.hh"
#include "sampler.hh"
#include "resource_pool.hh"
#include "common_resources.hh"

namespace lt::method
{

gamma::gamma(
    render_target& target,
    texture& src,
    resource_pool& pool,
    float g
):  target_method(target), src(&src), g(g),
    gamma_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "gamma.frag"}, {})
    ),
    quad(common::ensure_quad_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool))
{
}

void gamma::set_gamma(float g)
{
    this->g = g;
}

float gamma::get_gamma() const
{
    return g;
}

void gamma::execute()
{
    target_method::execute();

    if(!gamma_shader || !src) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    gamma_shader->bind();
    gamma_shader->set("gamma", 1.0f/g);
    gamma_shader->set("in_color", fb_sampler.bind(*src));

    quad.draw();
}

std::string gamma::get_name() const
{
    return "gamma";
}

} // namespace lt::method
