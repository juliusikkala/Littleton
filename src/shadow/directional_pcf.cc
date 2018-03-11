#include "directional_pcf.hh"
#include "object.hh"
#include "model.hh"
#include "vertex_buffer.hh"
#include "scene.hh"
#include "helpers.hh"
#include "shader_pool.hh"
#include <glm/gtc/random.hpp>

static texture* generate_shadow_kernel(context& ctx, unsigned size)
{
    std::vector<glm::vec2> kernel;

    mitchell_best_candidate(
        kernel,
        1.0f,
        20,
        size
    );

    return new texture(
        ctx,
        glm::uvec2(size),
        GL_RG8_SNORM,
        GL_FLOAT,
        0,
        GL_TEXTURE_1D,
        (float*)kernel.data()
    );
}

static texture* generate_shadow_noise_texture(context& ctx, glm::uvec2 size)
{
    // Just generate sines and cosines to rotate the kernel by.
    std::vector<glm::vec2> shadow_samples;
    shadow_samples.resize(size.x * size.y);

    for(unsigned i = 0; i < size.x * size.y; ++i)
    {
        shadow_samples[i] = glm::circularRand(1.0f);
    }

    return new texture(
        ctx,
        size,
        GL_RG8_SNORM,
        GL_FLOAT,
        0,
        GL_TEXTURE_2D,
        (float*)shadow_samples.data()
    );
}

directional_pcf_impl::directional_pcf_impl(context& ctx)
:   directional_shadow_map_impl(ctx),
    shadow_noise(generate_shadow_noise_texture(ctx, glm::uvec2(512))),
    kernel(generate_shadow_kernel(ctx, 256)),
    shadow_sampler(
        ctx,
        GL_LINEAR,
        GL_LINEAR,
        GL_CLAMP_TO_BORDER,
        0,
        glm::vec4(1),
        GL_COMPARE_REF_TO_TEXTURE
    ),
    noise_sampler(ctx, GL_NEAREST, GL_NEAREST, GL_REPEAT, 0)
{
}

void directional_pcf_impl::render(
    shader_pool& pool,
    const std::vector<directional_shadow_map*>& shadow_maps,
    render_scene* scene
){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    shader* depth_shader = pool.get(
        shader::path{"generic.vert", "empty.frag"},
        {{"VERTEX_POSITION", "0"}}
    );

    depth_shader->bind();

    for(directional_shadow_map* shadow_map: shadow_maps)
    {
        directional_shadow_map_pcf* pcf =
            dynamic_cast<directional_shadow_map_pcf*>(shadow_map);

        // Shadow should not be handled by this implementation!
        if(!pcf) continue;

        pcf->depth_buffer.bind();

        glClear(GL_DEPTH_BUFFER_BIT);

        glm::mat4 vp = pcf->get_projection() * pcf->get_view();

        for(object* obj: scene->get_objects())
        {
            model* mod = obj->get_model();
            if(!mod) continue;

            glm::mat4 mvp = vp * obj->get_global_transform();

            for(model::vertex_group& group: *mod)
            {
                if(!group.mesh) continue;

                depth_shader->set("mvp", mvp);
                group.mesh->draw();
            }
        }
    }
}

shader::definition_map directional_pcf_impl::get_definitions() const
{
    return {{"SHADOW_MAPPING", "shadow/directional_pcf.glsl"}};
}

void directional_pcf_impl::set_common_uniforms(
    shader* s,
    unsigned& texture_index
){
    s->set(
        "shadow_noise",
        noise_sampler.bind(*shadow_noise, texture_index++)
    );
    s->set("shadow_kernel", noise_sampler.bind(*kernel, texture_index++));
}

void directional_pcf_impl::set_shadow_map_uniforms(
    shader* s,
    unsigned& texture_index,
    directional_shadow_map* shadow_map,
    const std::string& prefix,
    const glm::mat4& pos_to_world
){
    directional_shadow_map_pcf* pcf =
        dynamic_cast<directional_shadow_map_pcf*>(shadow_map);
    if(!pcf) return;

    glm::mat4 lvp = pcf->get_projection() * pcf->get_view();

    s->set(
        prefix + "map",
        shadow_sampler.bind(pcf->depth, texture_index++)
    );
    s->set(prefix + "min_bias", pcf->min_bias);
    s->set(prefix + "max_bias", pcf->max_bias);
    s->set(prefix + "radius", pcf->radius);
    s->set(prefix + "mvp", lvp * pos_to_world);
    s->set<int>(prefix + "samples", (int)pcf->samples);
}

directional_shadow_map_pcf::directional_shadow_map_pcf(
    context& ctx,
    glm::uvec2 size,
    unsigned samples,
    float radius,
    glm::vec3 offset,
    glm::vec2 area,
    glm::vec2 depth_range,
    directional_light* light
):  directional_shadow_map(ctx, offset, area, depth_range, light),
    depth(
       ctx,
       size,
       GL_DEPTH_COMPONENT16,
       GL_FLOAT
    ),
    depth_buffer(ctx, size, {{GL_DEPTH_ATTACHMENT, {&depth}}}),
    radius(radius), samples(samples)
{
    set_bias();
}

directional_shadow_map_pcf::directional_shadow_map_pcf(
    directional_shadow_map_pcf&& other
):  directional_shadow_map(other),
    depth(std::move(other.depth)),
    depth_buffer(std::move(other.depth_buffer)),
    min_bias(other.min_bias), max_bias(other.max_bias),
    radius(other.radius), samples(other.samples)
{}

void directional_shadow_map_pcf::set_bias(float min_bias, float max_bias)
{
    this->min_bias = min_bias;
    this->max_bias = max_bias;
}

glm::vec2 directional_shadow_map_pcf::get_bias() const
{
    return glm::vec2(min_bias, max_bias);
}

void directional_shadow_map_pcf::set_samples(unsigned samples)
{
    this->samples = samples;
}

unsigned directional_shadow_map_pcf::get_samples() const
{
    return samples;
}

void directional_shadow_map_pcf::set_radius(float radius)
{
    this->radius = radius;
}

float directional_shadow_map_pcf::set_radius() const
{
    return radius;
}

texture& directional_shadow_map_pcf::get_depth()
{
    return depth;
}

const texture& directional_shadow_map_pcf::get_depth() const
{
    return depth;
}

bool directional_shadow_map_pcf::impl_is_compatible(
    const directional_shadow_map_impl* impl
){
    return dynamic_cast<const directional_pcf_impl*>(impl) != nullptr;
}

directional_pcf_impl* directional_shadow_map_pcf::create_impl() const
{
    return new directional_pcf_impl(depth.get_context());
}
