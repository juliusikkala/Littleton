#ifndef SHADOW_MSM_HH
#define SHADOW_MSM_HH
#include "shadow_map.hh"
#include "framebuffer.hh"
#include "texture.hh"
#include "sampler.hh"
#include "vertex_buffer.hh"
#include <memory>

class ms_render_target;
class pp_render_target;
class directional_msm_impl: public directional_shadow_map_impl
{
public:
    directional_msm_impl(resource_pool& pool);

    void render(
        const std::vector<directional_shadow_map*>& shadow_maps,
        render_scene* scene
    ) override;

    shader::definition_map get_definitions() const override;

    void set_common_uniforms(shader* s, unsigned& texture_index) override;

    void set_shadow_map_uniforms(
        shader* s,
        unsigned& texture_index,
        directional_shadow_map* shadow_map,
        const std::string& prefix,
        const glm::mat4& pos_to_world
    ) override;

private:
    void ensure_render_targets(
        const std::vector<directional_shadow_map*>& shadow_maps
    );

    // Multisampling render targets by sample count.
    std::map<unsigned, std::unique_ptr<ms_render_target>> ms_rt;

    // Postprocessing render target
    std::unique_ptr<pp_render_target> pp_rt;

    shader* depth_shader;
    shader* vertical_blur_shader;
    shader* horizontal_blur_shader;

    vertex_buffer quad;
    sampler moment_sampler;
};

class directional_shadow_map_msm: public directional_shadow_map
{
friend class directional_msm_impl;
public:
    directional_shadow_map_msm(
        context& ctx,
        glm::uvec2 size,
        unsigned samples = 16,
        unsigned radius = 4,
        glm::vec3 offset = glm::vec3(0),
        glm::vec2 area = glm::vec2(1.0f),
        glm::vec2 depth_range = glm::vec2(1.0f, -1.0f),
        directional_light* light = nullptr
    );

    directional_shadow_map_msm(directional_shadow_map_msm&& other);

    unsigned get_samples() const;

    void set_radius(unsigned radius);
    unsigned get_radius() const;

    texture& get_moments();
    const texture& get_moments() const;

    bool impl_is_compatible(const directional_shadow_map_impl* impl);
    directional_msm_impl* create_impl(resource_pool& pool) const;

private:
    texture moments;
    framebuffer moments_buffer;
    unsigned samples;
    unsigned radius;
};

#endif

