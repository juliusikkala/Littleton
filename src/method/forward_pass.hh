#ifndef METHOD_FORWARD_PASS_HH
#define METHOD_FORWARD_PASS_HH
#include "pipeline.hh"
#include "stencil_handler.hh"

class render_scene;
class resource_pool;
class multishader;
class shader;
class gbuffer;
class sampler;
class primitive;

namespace method
{
    class shadow_method;
    class forward_pass: public target_method, public stencil_handler
    {
    public:
        forward_pass(
            render_target& target,
            resource_pool& pool,
            render_scene* scene,
            bool apply_ambient = true,
            bool apply_transmittance = true
        );

        forward_pass(
            gbuffer& target,
            resource_pool& pool,
            render_scene* scene,
            bool apply_ambient = true,
            bool apply_transmittance = true
        );

        ~forward_pass();

        void set_scene(render_scene* s);
        render_scene* get_scene() const;

        // Whether to apply ambient lighting or not. Disable if you are using
        // SSAO or other methods which add the ambient term on their own.
        void set_apply_ambient(bool apply_ambient);
        bool get_apply_ambient() const;

        // Whether to apply object transmittance or not. Disable if you are
        // using SSRT or similar methods for refractions.
        void set_apply_transmittance(bool apply_transmittance);
        bool get_apply_transmittance() const;

        void render_opaque(bool opaque);
        void render_transparent(bool transparent);

        void execute() override;

        std::string get_name() const override;

    private:
        multishader* forward_shader;
        multishader* depth_shader;
        shader* min_max_shader;

        render_scene* scene; 
        gbuffer* gbuf;

        bool opaque;
        bool transparent;

        bool apply_ambient;
        bool apply_transmittance;
        const primitive& quad;
        const sampler& fb_sampler;
    };
}

#endif
