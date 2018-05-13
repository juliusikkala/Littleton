#ifndef LT_METHOD_GAMMA_HH
#define LT_METHOD_GAMMA_HH
#include "pipeline.hh"
#include "primitive.hh"

namespace lt
{

class shader_pool;
class texture;
class resource_pool;
class sampler;

}

namespace lt::method
{

class gamma: public target_method
{
public:
    gamma(
        render_target& target,
        texture& src,
        resource_pool& pool,
        float gamma = 2.2
    );

    void set_gamma(float gamma);
    float get_gamma() const;

    void execute() override;

    std::string get_name() const override;

private:
    texture* src;

    float g;
    shader* gamma_shader;

    const primitive& quad;
    const sampler& fb_sampler;
};

} // namespace lt::method

#endif

