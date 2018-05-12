#ifndef METHOD_BLOOM_HH
#define METHOD_BLOOM_HH
#include "pipeline.hh"
#include "primitive.hh"
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
            unsigned radius = 5,
            float strength = 1.0f,
            unsigned level = 2
        );

        void set_threshold(float threshold);
        float get_threshold() const;

        void set_radius(unsigned radius);
        unsigned get_radius() const;

        void set_strength(float strength);
        float get_strength() const;

        void set_level(unsigned level);
        unsigned get_level() const;

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
        unsigned level;
        std::vector<float> gaussian_kernel;

        const primitive& quad;
        sampler smooth_sampler;
    };
}
#endif
