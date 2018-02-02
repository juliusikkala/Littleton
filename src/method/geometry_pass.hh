#ifndef METHOD_GEOMETRY_PASS_HH
#define METHOD_GEOMETRY_PASS_HH
#include "pipeline.hh"
#include "gbuffer.hh"
#include "shader_cache.hh"
#include "scene.hh"

namespace method
{
    class geometry_pass: public pipeline_method
    {
    public:
        geometry_pass(
            gbuffer& buf,
            shader_cache* geometry_shader,
            render_scene* scene
        );

        void set_scene(render_scene* scene);
        render_scene* get_scene() const;

        void execute() override;

    private:
        shader_cache* geometry_shader;
        render_scene* scene;
    };
};
#endif
