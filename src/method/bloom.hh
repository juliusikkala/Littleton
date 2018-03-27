#ifndef METHOD_BLOOM_HH
#define METHOD_BLOOM_HH
#include "pipeline.hh"
#include "vertex_buffer.hh"
#include "sampler.hh"

class texture;
class resource_pool;
class multishader;

namespace method
{
    class bloom: public target_method
    {
    public:
        bloom(
            render_target& target,
            resource_pool& pool,
            texture* src,
            float threshold = 2.0f,
            unsigned radius = 10,
            float strength = 1.0f
        );

        void set_threshold(float threshold);
        float get_threshold() const;

        void set_radius(unsigned radius);
        unsigned get_radius() const;

        void set_strength(float strength);
        float get_strength() const;

        void execute() override;

        std::string get_name() const override;

    private:
        resource_pool& pool;

        texture* src;

        shader* threshold_shader;
        multishader* convolution_shader;
        shader* apply_shader;

        float threshold;
        unsigned radius;
        float strength;
        std::vector<float> gaussian_kernel;

        const vertex_buffer& quad;
        const sampler& fb_sampler;
    };
}
#endif
