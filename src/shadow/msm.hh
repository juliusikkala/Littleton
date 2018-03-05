#ifndef SHADOW_MSM_HH
#define SHADOW_MSM_HH
#include "shadow_map.hh"
#include "framebuffer.hh"
#include "texture.hh"
#include "vertex_buffer.hh"
#include <memory>

class ms_render_target;
class pp_render_target;
class msm_impl: public shadow_map_impl
{
public:
    msm_impl(context& ctx);

    void render(
        shader_store& store,
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
    void ensure_render_targets(
        const std::set<basic_shadow_map*>& shadow_maps
    );

    // Multisampling render targets by sample count.
    std::map<unsigned, std::unique_ptr<ms_render_target>> ms_rt;

    // Postprocessing render target
    std::unique_ptr<pp_render_target> pp_rt;

    vertex_buffer quad;
};

class msm_shadow_map
{
friend class msm_impl;
public:
    msm_shadow_map(
        context& ctx,
        glm::uvec2 size,
        unsigned samples = 4,
        unsigned radius = 4
    );
    msm_shadow_map(msm_shadow_map&& other);

    unsigned get_samples() const;

    void set_radius(unsigned radius);
    unsigned get_radius() const;

    texture& get_moments();
    const texture& get_moments() const;

    bool impl_is_compatible(const shadow_map_impl* impl);
    msm_impl* create_impl() const;

private:
    texture moments;
    framebuffer moments_buffer;
    unsigned samples;
    unsigned radius;
};

class directional_shadow_map_msm
: public basic_shadow_map, public msm_shadow_map, public directional_shadow_map
{
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

    bool impl_is_compatible(const shadow_map_impl* impl) override;
    msm_impl* create_impl() const override;

    light* get_light() const override;
    glm::mat4 get_view() const override;
    glm::mat4 get_projection() const override;
};

#endif

