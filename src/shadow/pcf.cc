#include "pcf.hh"
#include "object.hh"
#include "model.hh"
#include "vertex_buffer.hh"
#include "scene.hh"
#include "helpers.hh"
#include "shader_store.hh"
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
        GL_RG,
        GL_RG8_SNORM,
        GL_FLOAT,
        {false, GL_NEAREST, GL_REPEAT, 0},
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
        GL_RG,
        GL_RG8_SNORM,
        GL_FLOAT,
        {false, GL_LINEAR, GL_REPEAT, 0},
        GL_TEXTURE_2D,
        (float*)shadow_samples.data()
    );
}

pcf_impl::pcf_impl(context& ctx)
:   shadow_map_impl(ctx),
    shadow_noise(generate_shadow_noise_texture(ctx, glm::uvec2(512))),
    kernel(generate_shadow_kernel(ctx, 256))
{
}

void pcf_impl::render(
    shader_store& store,
    const std::set<basic_shadow_map*>& shadow_maps,
    render_scene* scene
){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    shader* depth_shader = store.get(
        shader::path{"generic.vert", "empty.frag"},
        {{"VERTEX_POSITION", "0"}}
    );

    depth_shader->bind();

    for(basic_shadow_map* shadow_map: shadow_maps)
    {
        pcf_shadow_map* pcf = dynamic_cast<pcf_shadow_map*>(shadow_map);
        if(!pcf) continue;

        pcf->depth_buffer.bind();

        glClear(GL_DEPTH_BUFFER_BIT);

        glm::mat4 vp = shadow_map->get_projection() * shadow_map->get_view();

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

shader::definition_map pcf_impl::get_definitions() const
{
    return {{"SHADOW_IMPLEMENTATION", "shadow/pcf.glsl"}};
}

void pcf_impl::set_common_uniforms(shader* s, unsigned& texture_index)
{
    s->set("shadow_noise", shadow_noise->bind(texture_index++));
    s->set("shadow_kernel", kernel->bind(texture_index++));
}

void pcf_impl::set_shadow_map_uniforms(
    shader* s,
    unsigned& texture_index,
    basic_shadow_map* shadow_map,
    const std::string& prefix,
    const glm::mat4& pos_to_world
){
    pcf_shadow_map* pcf = dynamic_cast<pcf_shadow_map*>(shadow_map);
    if(!pcf) return;

    glm::mat4 lvp = shadow_map->get_projection() * shadow_map->get_view();

    s->set(prefix + "map", pcf->depth.bind(texture_index++));
    s->set(prefix + "min_bias", pcf->min_bias);
    s->set(prefix + "max_bias", pcf->max_bias);
    s->set(prefix + "radius", pcf->radius);
    s->set(prefix + "mvp", lvp * pos_to_world);
    s->set<int>(prefix + "samples", (int)pcf->samples);
}

pcf_shadow_map::pcf_shadow_map(
    context& ctx,
    glm::uvec2 size,
    unsigned samples,
    float radius
):  depth(
       ctx,
       size,
       GL_DEPTH_COMPONENT,
       GL_DEPTH_COMPONENT16,
       GL_FLOAT,
       texture::params(false, GL_LINEAR, GL_CLAMP_TO_BORDER, 0, glm::vec4(1))
    ),
    depth_buffer(ctx, size),
    radius(radius), samples(samples)
{
    depth_buffer.set_depth_target(&depth);
    set_bias();
}

pcf_shadow_map::pcf_shadow_map(pcf_shadow_map&& other)
:   depth(std::move(other.depth)),
    depth_buffer(std::move(other.depth_buffer)),
    min_bias(other.min_bias), max_bias(other.max_bias),
    radius(other.radius), samples(other.samples)
{}

void pcf_shadow_map::set_bias(float min_bias, float max_bias)
{
    this->min_bias = min_bias;
    this->max_bias = max_bias;
}

glm::vec2 pcf_shadow_map::get_bias() const
{
    return glm::vec2(min_bias, max_bias);
}

void pcf_shadow_map::set_samples(unsigned samples)
{
    this->samples = samples;
}

unsigned pcf_shadow_map::get_samples() const
{
    return samples;
}

void pcf_shadow_map::set_radius(float radius)
{
    this->radius = radius;
}

float pcf_shadow_map::set_radius() const
{
    return radius;
}

texture& pcf_shadow_map::get_depth()
{
    return depth;
}

const texture& pcf_shadow_map::get_depth() const
{
    return depth;
}

bool pcf_shadow_map::impl_is_compatible(const shadow_map_impl* impl)
{
    return dynamic_cast<const pcf_impl*>(impl) != nullptr;
}

pcf_impl* pcf_shadow_map::create_impl() const
{
    return new pcf_impl(depth.get_context());
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
):  basic_shadow_map(ctx),
    pcf_shadow_map(ctx, size, samples, radius),
    directional_shadow_map(offset, area, depth_range, light)
{}

bool directional_shadow_map_pcf::impl_is_compatible(const shadow_map_impl* impl)
{ return pcf_shadow_map::impl_is_compatible(impl); }

pcf_impl* directional_shadow_map_pcf::create_impl() const
{ return pcf_shadow_map::create_impl(); }

light* directional_shadow_map_pcf::get_light() const
{ return directional_shadow_map::get_light(); }

glm::mat4 directional_shadow_map_pcf::get_view() const
{ return directional_shadow_map::get_view(); }

glm::mat4 directional_shadow_map_pcf::get_projection() const
{ return directional_shadow_map::get_projection(); }
