#include "bloom.hh"
#include "resource_pool.hh"
#include "shader.hh"
#include "multishader.hh"
#include "common_resources.hh"
#include "helpers.hh"

method::bloom::bloom(
    render_target& target,
    resource_pool& pool,
    texture* src,
    float threshold,
    unsigned radius,
    float strength
):  target_method(target), pool(pool), src(src),
    threshold_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "threshold.frag"}, {}
    )),
    convolution_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "convolution.frag"}
    )),
    apply_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "blend_texture.frag"}, {}
    )),
    threshold(threshold), radius(radius), strength(strength),
    quad(common::ensure_quad_vertex_buffer(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool))
{
    set_radius(radius);
}

void method::bloom::set_threshold(float threshold)
{
    this->threshold = threshold;
}

float method::bloom::get_threshold() const
{
    return threshold;
}

void method::bloom::set_radius(unsigned radius)
{
    this->radius = radius;

    gaussian_kernel = generate_gaussian_kernel(radius, 0.4f * radius);
}

unsigned method::bloom::get_radius() const
{
    return radius;
}

void method::bloom::set_strength(float strength)
{
    this->strength = strength;
}

float method::bloom::get_strength() const
{
    return strength;
}

void method::bloom::execute()
{
    if(radius <= 0.0f || strength <= 0.0f || !src)
        return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    auto tmp1_fb = pool.loan_framebuffer(
        get_target().get_size(),
        {{GL_COLOR_ATTACHMENT0, {src->get_internal_format(), true}}}
    );

    auto tmp2_fb = pool.loan_framebuffer(
        get_target().get_size(),
        {{GL_COLOR_ATTACHMENT0, {src->get_internal_format(), true}}}
    );

    tmp1_fb->bind();

    threshold_shader->bind();

    threshold_shader->set("src_color", fb_sampler.bind(*src));
    threshold_shader->set("threshold", threshold);
    threshold_shader->set("fail_color", glm::vec4(0,0,0,1));

    quad.draw();

    tmp2_fb->bind();

    shader* cs = convolution_shader->get(
        {{"MAX_CONVOLUTION_SIZE", std::to_string(radius * 2 + 1)}}
    );

    cs->bind();
    cs->set(
        "src",
        fb_sampler.bind(*tmp1_fb->get_texture_target(GL_COLOR_ATTACHMENT0))
    );
    cs->set("kernel", gaussian_kernel.size(), gaussian_kernel.data());
    cs->set("convolution_size", (int)(2 * radius + 1));
    cs->set("step_size", glm::ivec2(0,2));

    quad.draw();

    tmp1_fb->bind();

    cs->set(
        "src",
        fb_sampler.bind(*tmp2_fb->get_texture_target(GL_COLOR_ATTACHMENT0))
    );
    cs->set("step_size", glm::ivec2(2,0));
    quad.draw();

    get_target().bind();

    apply_shader->bind();
    apply_shader->set("src1", fb_sampler.bind(*src, 0));
    apply_shader->set(
        "src2",
        fb_sampler.bind(*tmp1_fb->get_texture_target(GL_COLOR_ATTACHMENT0), 1)
    );
    apply_shader->set("multiplier1", 1.0f);
    apply_shader->set("multiplier2", strength);

    quad.draw();
}

std::string method::bloom::get_name() const
{
    return "bloom";
}