#ifndef SHADOW_DIRECTIONAL_SHADOW_MAP_PCF_HH
#define SHADOW_DIRECTIONAL_SHADOW_MAP_PCF_HH
#include "shadow_map.hh"
#include "framebuffer.hh"

class directional_shadow_map_pcf: public directional_shadow_map
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
    directional_shadow_map_pcf(directional_shadow_map_pcf&& other);
    ~directional_shadow_map_pcf();

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

    virtual void render(
        shader_store& store,
        render_scene* scene,
        shared_resources* resources
    ) override;

    virtual bool merge_shared_resources(shared_resources* res) const override;
    virtual shared_resources* create_shared_resources() const override;

    virtual void set_uniforms(
        shader* s,
        const std::string& prefix,
        unsigned& texture_index,
        const glm::mat4& pos_to_world
    ) override;

private:
    texture depth;
    framebuffer depth_buffer;
    float min_bias, max_bias;
    float radius;
    unsigned samples;

    class pcf_shared_resources: public shared_resources, public glresource
    {
    public:
        pcf_shared_resources(context& ctx, unsigned kernel_size);

        void set_kernel_size(unsigned kernel_size);
        unsigned get_kernel_size() const;

        virtual shader::definition_map get_definitions() const override;
        virtual void set_uniforms(shader* s, unsigned& texture_index) override;

    private:
        void generate_resources();

        unsigned kernel_size;
        std::unique_ptr<texture> shadow_noise;
        std::vector<glm::vec2> shadow_kernel;
    };
};

#endif
