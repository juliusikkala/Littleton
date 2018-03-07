#ifndef SHADOW_PCF_HH
#define SHADOW_PCF_HH
#include "shadow_map.hh"
#include "framebuffer.hh"
#include "texture.hh"
#include <memory>

class pcf_impl: public shadow_map_impl
{
public:
    pcf_impl(context& ctx);

    void render(
        shader_pool& store,
        const std::set<basic_shadow_map*>& shadow_maps,
        render_scene* scene
    ) override;

    shader::definition_map get_definitions() const override;

    void set_common_uniforms(shader* s, unsigned& texture_index) override;

    void set_shadow_map_uniforms(
        shader* s,
        unsigned& texture_index,
        basic_shadow_map* shadow_map,
        const std::string& prefix,
        const glm::mat4& pos_to_world
    ) override;

private:
    std::unique_ptr<texture> shadow_noise;
    std::unique_ptr<texture> kernel;
};

class pcf_shadow_map
{
friend class pcf_impl;
public:
    pcf_shadow_map(
        context& ctx,
        glm::uvec2 size,
        unsigned samples = 16,
        float radius = 4.0f
    );
    pcf_shadow_map(pcf_shadow_map&& other);

    void set_bias(
        float min_bias = 0.001,
        float max_bias = 0.02
    );

    glm::vec2 get_bias() const;

    void set_samples(unsigned samples);
    unsigned get_samples() const;

    void set_radius(float radius);
    float set_radius() const;

    texture& get_depth();
    const texture& get_depth() const;

    bool impl_is_compatible(const shadow_map_impl* impl);
    pcf_impl* create_impl() const;

private:
    texture depth;
    framebuffer depth_buffer;
    float min_bias, max_bias;
    float radius;
    unsigned samples;
};

class directional_shadow_map_pcf
: public basic_shadow_map, public pcf_shadow_map, public directional_shadow_map
{
public:
    directional_shadow_map_pcf(
        context& ctx,
        glm::uvec2 size,
        unsigned samples = 16,
        float radius = 4.0f,
        glm::vec3 offset = glm::vec3(0),
        glm::vec2 area = glm::vec2(1.0f),
        glm::vec2 depth_range = glm::vec2(1.0f, -1.0f),
        directional_light* light = nullptr
    );

    bool impl_is_compatible(const shadow_map_impl* impl) override;
    pcf_impl* create_impl() const override;

    light* get_light() const override;
    glm::mat4 get_view() const override;
    glm::mat4 get_projection() const override;
};

#endif
