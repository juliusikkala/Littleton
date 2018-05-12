#ifndef METHOD_SSRT_HH
#define METHOD_SSRT_HH
#include "pipeline.hh"
#include "primitive.hh"
#include "sampler.hh"
#include "doublebuffer.hh"
#include "resource.hh"
#include "stencil_handler.hh"
#include <memory>

class gbuffer;
class resource_pool;
class multishader;
class shader;
class render_scene;

namespace method
{
    class ssrt: public target_method, public stencil_handler
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
        // Distance of the first sample from the point where the ray starts
        void set_ray_offset(float offset = 0.01f);

        void use_fallback_cubemap(bool use = true);

        void execute() override;

        std::string get_name() const override;

    private:
        void refresh_shader();

        gbuffer* buf;
        resource_pool& pool;

        multishader* ssrt_shaders;
        shader* ssrt_shader;
        shader* blit_shader;

        render_scene* scene;

        const primitive& quad;
        const sampler& fb_sampler;
        sampler mipmap_sampler;
        sampler cubemap_sampler;

        unsigned max_steps;
        float thickness;
        float roughness_cutoff;
        float brdf_cutoff;
        float ray_offset;
        bool fallback_cubemap;
    };
}

#endif
