#ifndef METHOD_LIGHTING_PASS_HH
#define METHOD_LIGHTING_PASS_HH
#include "pipeline.hh"
#include "vertex_buffer.hh"
#include "sampler.hh"

class gbuffer;
class resource_pool;
class render_scene;
class multishader;

namespace method
{
    class lighting_pass: public target_method
    {
    public:
        lighting_pass(
            render_target& target,
            gbuffer& buf,
            resource_pool& pool,
            render_scene* scene
        );

        void set_scene(render_scene* scene);
        render_scene* get_scene() const;

        void execute() override;

        std::string get_name() const override;

    private:
        gbuffer* buf;

        multishader* lighting_shader;
        render_scene* scene;

        const vertex_buffer& quad;
        const sampler& fb_sampler;
    };
}

#endif
