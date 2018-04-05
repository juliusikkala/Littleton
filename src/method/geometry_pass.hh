#ifndef METHOD_GEOMETRY_PASS_HH
#define METHOD_GEOMETRY_PASS_HH
#include "pipeline.hh"

class gbuffer;
class sampler;
class render_scene;
class multishader;
class shader;
class vertex_buffer;
class resource_pool;

namespace method
{
    class geometry_pass: public target_method
    {
    public:
        geometry_pass(
            gbuffer& buf,
            resource_pool& store,
            render_scene* scene
        );

        void set_scene(render_scene* scene);
        render_scene* get_scene() const;

        void execute() override;

        std::string get_name() const override;

    private:
        multishader* geometry_shader;
        shader* min_max_shader;
        render_scene* scene;
        const vertex_buffer& quad;
        const sampler& fb_sampler;
    };
};
#endif
