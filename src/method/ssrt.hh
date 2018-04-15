#ifndef METHOD_SSRT_HH
#define METHOD_SSRT_HH
#include "pipeline.hh"
#include "vertex_buffer.hh"
#include "sampler.hh"
#include "doublebuffer.hh"
#include "resource.hh"
#include <memory>

class gbuffer;
class resource_pool;
class multishader;
class shader;
class render_scene;

namespace method
{
    class ssrt: public target_method
    {
    public:
        ssrt(
            render_target& target,
            gbuffer& buf,
            resource_pool& pool,
            render_scene* scene
        );

        // Limiting max steps improves worst-case performance, but limits
        // reflected range near geometry boundaries. When thickness is finite,
        // this should be larger. 
        void set_max_steps(unsigned max_steps = 500);
        void set_roughness_cutoff(float cutoff = 0.5f);
        void set_brdf_cutoff(float cutoff = 0.0f);
        // Set to negative for infinite depth (faster)
        void set_thickness(float thickness = -1.0f);

        void execute() override;

        std::string get_name() const override;

    private:
        gbuffer* buf;
        resource_pool& pool;

        multishader* ssrt_shaders;
        shader* ssrt_shader;
        shader* blit_shader;

        render_scene* scene;

        const vertex_buffer& quad;
        const sampler& fb_sampler;
        sampler mipmap_sampler;

        unsigned max_steps;
        float thickness;
        float roughness_cutoff;
        float brdf_cutoff;
    };
}

#endif
