#ifndef LT_METHOD_GEOMETRY_PASS_HH
#define LT_METHOD_GEOMETRY_PASS_HH
#include "pipeline.hh"
#include "stencil_handler.hh"

namespace lt
{

class gbuffer;
class sampler;
class render_scene;
class multishader;
class shader;
class primitive;
class resource_pool;

}

namespace lt::method
{

class geometry_pass: public target_method, public stencil_handler
{
public:
    geometry_pass(
        gbuffer& buf,
        resource_pool& store,
        render_scene* scene,
        bool apply_ambient = true
    );

    void set_scene(render_scene* scene);
    render_scene* get_scene() const;

    void set_apply_ambient(bool apply_ambient);
    bool get_apply_ambient() const;

    void execute() override;

    std::string get_name() const override;

private:
    multishader* geometry_shader;
    shader* min_max_shader;
    render_scene* scene;
    const primitive& quad;
    const sampler& fb_sampler;

    bool apply_ambient;
};

} // namespace lt::method

#endif
