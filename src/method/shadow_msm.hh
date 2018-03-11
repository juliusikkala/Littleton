#ifndef METHOD_SHADOW_MSM_HH
#define METHOD_SHADOW_MSM_HH
#include "pipeline.hh"
#include "render_target.hh"
#include "shadow_method.hh"
#include "texture.hh"
#include "framebuffer.hh"
#include "sampler.hh"

class resource_pool;
class shader;
class vertex_buffer;
namespace method { class shadow_msm; }

class directional_shadow_map_msm: public directional_shadow_map
{
friend class method::shadow_msm;
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

private:
    texture moments;
    framebuffer moments_buffer;
    unsigned samples;
    unsigned radius;
};

namespace method
{
    class shadow_msm: public glresource, public shadow_method
    {
    public:
        shadow_msm(resource_pool& pool, render_scene* scene);

        void add(directional_shadow_map_msm* shadow_map);
        void remove(directional_shadow_map_msm* shadow_map);
        void clear();

        shader::definition_map get_directional_definitions() const override;

        size_t get_directional_shadow_map_count() const override;

        directional_shadow_map_msm*
        get_directional_shadow_map(unsigned i) const override;

        void set_directional_shadow_map_uniforms(
            shader* s,
            unsigned& texture_index,
            unsigned i,
            const std::string& prefix,
            const glm::mat4& pos_to_world
        ) override;

        void execute() override;

        std::string get_name() const override;

    private:
        void ensure_render_targets();

        // Multisampling render targets by sample count.
        std::map<unsigned, std::unique_ptr<framebuffer>> ms_rt;

        // Postprocessing render target
        std::unique_ptr<framebuffer> pp_rt;

        shader* depth_shader;
        shader* vertical_blur_shader;
        shader* horizontal_blur_shader;

        const vertex_buffer& quad;
        sampler moment_sampler;

        std::vector<directional_shadow_map_msm*> directional_shadow_maps;
    };
};

#endif
