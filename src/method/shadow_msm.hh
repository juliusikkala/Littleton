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
        method::shadow_msm* method,
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

    framebuffer& get_framebuffer();
    const framebuffer& get_framebuffer() const;

private:
    texture moments;
    framebuffer moments_buffer;
    unsigned samples;
    unsigned radius;
};

class omni_shadow_map_msm: public omni_shadow_map
{
friend class method::shadow_msm;
public:
    omni_shadow_map_msm(
        method::shadow_msm* method,
        context& ctx,
        glm::uvec2 size,
        unsigned samples = 4,
        // Currently unused due to difficulties in cube map blurring
        float radius = 1,
        glm::vec2 depth_range = glm::vec2(0.01f, 10.0f),
        point_light* light = nullptr
    );
    omni_shadow_map_msm(omni_shadow_map_msm&& other);

    unsigned get_samples() const;

    texture& get_moments();
    const texture& get_moments() const;

    framebuffer& get_framebuffer();
    const framebuffer& get_framebuffer() const;

private:
    texture moments;
    framebuffer moments_buffer;
    unsigned samples;
};

class perspective_shadow_map_msm: public perspective_shadow_map
{
friend class method::shadow_msm;
public:
    perspective_shadow_map_msm(
        method::shadow_msm* method,
        context& ctx,
        glm::uvec2 size,
        unsigned samples = 16,
        unsigned radius = 4,
        double fov = 30,
        glm::vec2 depth_range = glm::vec2(0.01f, 10.0f),
        point_light* light = nullptr
    );

    perspective_shadow_map_msm(perspective_shadow_map_msm&& other);

    unsigned get_samples() const;

    void set_radius(unsigned radius);
    unsigned get_radius() const;

    texture& get_moments();
    const texture& get_moments() const;

    framebuffer& get_framebuffer();
    const framebuffer& get_framebuffer() const;

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

        shader::definition_map get_directional_definitions() const override;
        shader::definition_map get_omni_definitions() const override;
        shader::definition_map get_perspective_definitions() const override;

        void set_shadow_map_uniforms(
            shader* s,
            unsigned& texture_index,
            directional_shadow_map* shadow_map,
            const std::string& prefix,
            const glm::mat4& pos_to_world
        ) override;

        void set_shadow_map_uniforms(
            shader* s,
            unsigned& texture_index,
            omni_shadow_map* shadow_map,
            const std::string& prefix,
            const glm::mat4& pos_to_world
        ) override;

        void set_shadow_map_uniforms(
            shader* s,
            unsigned& texture_index,
            perspective_shadow_map* shadow_map,
            const std::string& prefix,
            const glm::mat4& pos_to_world
        ) override;

        void execute() override;

        std::string get_name() const override;

    private:
        resource_pool& pool;

        shader* depth_shader;
        shader* cubemap_depth_shader;
        shader* perspective_depth_shader;

        shader* vertical_blur_shader;
        shader* horizontal_blur_shader;

        const vertex_buffer& quad;
        sampler moment_sampler;
        sampler cubemap_moment_sampler;
    };
};

#endif
