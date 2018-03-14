#ifndef METHOD_SHADOW_PCF_HH
#define METHOD_SHADOW_PCF_HH
#include "pipeline.hh"
#include "render_target.hh"
#include "shadow_method.hh"
#include "texture.hh"
#include "framebuffer.hh"
#include "sampler.hh"

class directional_shadow_map_pcf;
class resource_pool;
class shader;
class vertex_buffer;
namespace method { class shadow_pcf; }

class directional_shadow_map_pcf: public directional_shadow_map
{
friend class method::shadow_pcf;
public:
    directional_shadow_map_pcf(
        method::shadow_pcf* method,
        context& ctx,
        glm::uvec2 size,
        unsigned samples = 16,
        float radius = 4.0f,
        glm::vec3 offset = glm::vec3(0),
        glm::vec2 area = glm::vec2(1.0f),
        glm::vec2 depth_range = glm::vec2(1.0f, -1.0f),
        directional_light* light = nullptr
    );
    directional_shadow_map_pcf(directional_shadow_map_pcf&& other);

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

private:
    texture depth;
    framebuffer depth_buffer;
    float min_bias, max_bias;
    float radius;
    unsigned samples;
};

class point_shadow_map_pcf: public point_shadow_map
{
friend class method::shadow_pcf;
public:
    point_shadow_map_pcf(
        method::shadow_pcf* method,
        context& ctx,
        glm::uvec2 size,
        unsigned samples = 16,
        float radius = 0.1f,
        glm::vec2 depth_range = glm::vec2(0.01f, 10.0f),
        point_light* light = nullptr
    );
    point_shadow_map_pcf(point_shadow_map_pcf&& other);

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

private:
    texture depth;
    framebuffer depth_buffer;
    float min_bias, max_bias;
    float radius;
    unsigned samples;
};

namespace method
{
    class shadow_pcf: public shadow_method
    {
    public:
        shadow_pcf(resource_pool& pool, render_scene* scene);

        void set_directional_uniforms(
            shader* s,
            unsigned& texture_index
        ) override;

        void set_point_uniforms(
            shader* s,
            unsigned& texture_index
        ) override;

        shader::definition_map get_directional_definitions() const override;
        shader::definition_map get_point_definitions() const override;

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
            point_shadow_map* shadow_map,
            const std::string& prefix,
            const glm::mat4& pos_to_world
        ) override;

        void execute() override;

        std::string get_name() const override;

    private:
        shader* depth_shader;
        shader* cubemap_depth_shader;
        const texture& shadow_noise_2d;
        const texture& shadow_noise_3d;
        const texture& kernel;
        sampler shadow_sampler, cubemap_shadow_sampler, noise_sampler;
    };
};
#endif
