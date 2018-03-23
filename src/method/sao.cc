#include "sao.hh"
#include "shader.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include "resource_pool.hh"
#include "camera.hh"
#include "scene.hh"
#include "common_resources.hh"
#include <glm/gtc/random.hpp>
#include <cmath>

method::sao::sao(
    render_target& target,
    gbuffer& buf,
    resource_pool& pool,
    render_scene* scene,
    float radius,
    unsigned samples,
    float bias,
    float intensity
):  target_method(target), glresource(pool.get_context()), buf(&buf),
    z_pass_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "sao/z_pass.frag"}, {}
    )),
    ao_sample_pass_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "sao/ao_sample_pass.frag"},
        {{"USE_NORMAL_TEXTURE", ""}}
    )),
    blur_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "sao/blur.frag"}, {}
    )),
    ambient_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "ambient.frag"}, {}
    )),
    scene(scene),
    radius(radius), samples(samples), bias(bias), intensity(intensity),
    linear_depth(get_context(), target.get_size(), {
        {GL_COLOR_ATTACHMENT0, {GL_R32F, true}}
    }),
    ao(get_context(), target.get_size(), GL_R8),
    quad(common::ensure_quad_vertex_buffer(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool)),
    mipmap_sampler(
        get_context(),
        GL_NEAREST,
        GL_NEAREST_MIPMAP_NEAREST,
        GL_CLAMP_TO_EDGE
    )
{
}

void method::sao::set_radius(float radius)
{
    this->radius = radius;
}

float method::sao::get_radius() const
{
    return radius;
}

void method::sao::set_samples(unsigned samples)
{
    this->samples = samples;
}

unsigned method::sao::get_samples() const
{
    return samples;
}

void method::sao::set_bias(float bias)
{
    this->bias = bias;
}

float method::sao::get_bias() const
{
    return bias;
}

void method::sao::set_intensity(float intensity)
{
    this->intensity = intensity;
}

float method::sao::get_intensity() const
{
    return intensity;
}

void method::sao::execute()
{
    if(!z_pass_shader || !scene || scene->get_ambient() == glm::vec3(0))
        return;

    camera* cam = scene->get_camera();
    if(!cam) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDisable(GL_STENCIL_TEST);

    // Hierarchical z Pass
    linear_depth.bind();

    z_pass_shader->bind();
    z_pass_shader->set("in_depth", fb_sampler.bind(buf->get_depth_stencil()));
    z_pass_shader->set("clip_info", cam->get_clip_info());

    quad.draw();

    texture* depth_tex = linear_depth.get_texture_target(GL_COLOR_ATTACHMENT0);
    depth_tex->generate_mipmaps();

    // Distributed AO sample pass
    ao.input().bind();

    glm::vec2 ppu = cam->pixels_per_unit(get_target().get_size());

    ao_sample_pass_shader->bind();
    ao_sample_pass_shader->set("in_depth", mipmap_sampler.bind(*depth_tex, 0));
    ao_sample_pass_shader->set(
        "in_normal",
        fb_sampler.bind(buf->get_normal(), 1)
    );
    ao_sample_pass_shader->set("proj_scale", ppu.y);
    ao_sample_pass_shader->set("radius", radius);
    ao_sample_pass_shader->set<int>("samples", samples);
    ao_sample_pass_shader->set("bias", bias);
    ao_sample_pass_shader->set<float>(
        "multiplier",
        5.0f * intensity / (samples * pow(radius, 6))
    );
    ao_sample_pass_shader->set("projection_info", cam->get_projection_info());
    quad.draw();

    // Blur passes
    ao.swap();
    ao.input().bind();

    blur_shader->bind();
    blur_shader->set("in_ao", fb_sampler.bind(ao.output(), 0));
    blur_shader->set("in_depth", fb_sampler.bind(*depth_tex, 1));
    blur_shader->set("step_size", glm::ivec2(1, 0));

    quad.draw();

    ao.swap();
    ao.input().bind();

    blur_shader->set("in_ao", fb_sampler.bind(ao.output(), 0));
    blur_shader->set("step_size", glm::ivec2(0, 1));

    quad.draw();

    // Apply
    glEnable(GL_BLEND);
    ao.swap();
    get_target().bind();

    ambient_shader->bind();
    ambient_shader->set(
        "in_color_emission",
        fb_sampler.bind(buf->get_color_emission(), 0)
    );

    ambient_shader->set("ambient", scene->get_ambient());
    ambient_shader->set("occlusion", fb_sampler.bind(ao.output(), 1));

    quad.draw();
}

std::string method::sao::get_name() const
{
    return "sao";
}
