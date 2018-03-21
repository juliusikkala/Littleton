#ifndef METHOD_LIGHTING_PASS_HH
#define METHOD_LIGHTING_PASS_HH
#include "pipeline.hh"
#include "vertex_buffer.hh"
#include "sampler.hh"
#include "shadow_method.hh"

class gbuffer;
class resource_pool;
class render_scene;
class multishader;

namespace method
{
    class shadow_method;
    class lighting_pass: public target_method
    {
    public:
        lighting_pass(
            render_target& target,
            gbuffer& buf,
            resource_pool& pool,
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
        gbuffer* buf;

        multishader* lighting_shader;
        render_scene* scene;

        bool apply_ambient;

        const vertex_buffer& quad;
        const sampler& fb_sampler;
    };
}

#endif
