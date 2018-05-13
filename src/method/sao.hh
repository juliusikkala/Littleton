#ifndef LT_METHOD_SAO_HH
#define LT_METHOD_SAO_HH
#include "pipeline.hh"
#include "primitive.hh"
#include "sampler.hh"
#include "shadow_method.hh"
#include "framebuffer.hh"
#include "doublebuffer.hh"
#include "resource.hh"
#include <memory>

namespace lt
{

class gbuffer;
class resource_pool;
class shader;

}

namespace lt::method
{

// Scalable ambient obsurance (McGuire, HPG 2012)
class sao: public target_method, public glresource
{
public:
    sao(
        render_target& target,
        gbuffer& buf,
        resource_pool& pool,
        render_scene* scene,
        float radius = 0.5f,
        unsigned samples = 8,
        float bias = 0.01f,
        float intensity = 1.0f
    );

    void set_radius(float radius);
    float get_radius() const;

    void set_samples(unsigned samples);
    unsigned get_samples() const;

    void set_bias(float bias);
    float get_bias() const;

    void set_intensity(float intensity);
    float get_intensity() const;

    void execute() override;

    std::string get_name() const override;

private:
    gbuffer* buf;

    shader* ao_sample_pass_shader;
    shader* blur_shader;
    shader* ambient_shader;
    render_scene* scene;

    float radius;
    unsigned samples;
    float bias;
    float intensity;
    float spiral_turns;

    doublebuffer ao;

    const primitive& quad;
    const sampler& fb_sampler;
    sampler mipmap_sampler;
};

} // namespace lt::method

#endif

