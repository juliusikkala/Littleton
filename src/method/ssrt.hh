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

        void execute() override;

        std::string get_name() const override;

    private:
        gbuffer* buf;
        resource_pool& pool;

        shader* ssrt_shader;
        shader* blit_shader;

        render_scene* scene;

        const vertex_buffer& quad;
        const sampler& fb_sampler;
    };
}

#endif
