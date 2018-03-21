#ifndef METHOD_SSAO_HH
#define METHOD_SSAO_HH
#include "pipeline.hh"
#include "vertex_buffer.hh"
#include "sampler.hh"
#include "shadow_method.hh"
#include "doublebuffer.hh"

class gbuffer;
class resource_pool;
class shader;

namespace method
{
    class ssao: public target_method
    {
    public:
        ssao(
            render_target& target,
            gbuffer& buf,
            resource_pool& pool,
            render_scene* scene,
            float radius = 0.2f,
            unsigned samples = 16,
            unsigned blur_radius = 1,
            float bias = 0.01f
        );

        void set_radius(float radius);
        float get_radius() const;

        void set_samples(unsigned samples);
        unsigned get_samples() const;

        void set_blur(unsigned blur_radius);
        unsigned get_blur() const;

        void set_bias(float bias);
        float get_bias() const;

        void execute() override;

        std::string get_name() const override;

    private:
        gbuffer* buf;

        shader* ssao_shader;
        shader* vertical_blur_shader;
        shader* horizontal_blur_shader;
        shader* ambient_shader;

        render_scene* scene;

        doublebuffer ssao_buffer;

        float radius;
        unsigned samples;
        unsigned blur_radius;
        float bias;

        const texture& random_rotation;
        texture kernel;

        const vertex_buffer& quad;
        const sampler& fb_sampler;
        sampler noise_sampler;
    };
}

#endif
