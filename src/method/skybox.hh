#ifndef METHOD_SKYBOX_HH
#define METHOD_SKYBOX_HH
#include "pipeline.hh"
#include "sampler.hh"

class shader;
class resource_pool;
class render_scene;
class vertex_buffer;

namespace method
{
    class skybox: public target_method
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

        const vertex_buffer& quad;
    };
}
#endif
