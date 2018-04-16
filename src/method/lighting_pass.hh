#ifndef METHOD_LIGHTING_PASS_HH
#define METHOD_LIGHTING_PASS_HH
#include "pipeline.hh"
#include "vertex_buffer.hh"
#include "sampler.hh"
#include "shadow_method.hh"
#include "stencil_handler.hh"

class gbuffer;
class resource_pool;
class render_scene;
class multishader;

namespace method
{
    class shadow_method;
    class lighting_pass: public target_method, public stencil_handler
    {
    public:
        lighting_pass(
            render_target& target,
            gbuffer& buf,
            resource_pool& pool,
            render_scene* scene,
            bool apply_ambient = true,
            float cutoff = 5/256.0f // Set to negative to not use light volumes
        );

        void set_scene(render_scene* scene);
        render_scene* get_scene() const;

        void set_cutoff(float cutoff);
        float get_cutoff() const;

        enum depth_test
        {
            TEST_NONE = 0,
            TEST_NEAR,
            TEST_FAR
        };

        void set_light_depth_test(depth_test test);
        depth_test get_light_depth_test() const;

        void set_apply_ambient(bool apply_ambient);
        bool get_apply_ambient() const;

        void set_visualize_light_volumes(bool visualize);

        void execute() override;

        std::string get_name() const override;

    private:
        gbuffer* buf;

        multishader* lighting_shader;
        render_scene* scene;

        bool apply_ambient;
        float cutoff;
        depth_test light_test;
        bool visualize_light_volumes;

        const vertex_buffer& quad;
        const sampler& fb_sampler;
    };
}

#endif
