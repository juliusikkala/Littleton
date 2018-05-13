#ifndef LT_METHOD_SKYBOX_HH
#define LT_METHOD_SKYBOX_HH
#include "pipeline.hh"
#include "sampler.hh"
#include "stencil_handler.hh"

namespace lt
{

class shader;
class resource_pool;
class render_scene;
class primitive;

}

namespace lt::method
{

class skybox: public target_method, public stencil_handler
{
public:
    skybox(
        render_target& target,
        resource_pool& pool,
        render_scene* scene = nullptr
    );

    void set_scene(render_scene* s);
    render_scene* get_scene() const;

    void set_exposure(float exposure);
    float get_exposure() const;

    void execute() override;

    std::string get_name() const override;

private:
    shader* sky_shader;
    render_scene* scene;
    sampler skybox_sampler;
    float exposure;

    const primitive& quad;
};

} // namespace lt::method

#endif
