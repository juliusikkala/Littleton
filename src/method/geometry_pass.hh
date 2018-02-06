#ifndef METHOD_GEOMETRY_PASS_HH
#define METHOD_GEOMETRY_PASS_HH
#include "pipeline.hh"
#include "gbuffer.hh"
#include "multishader.hh"
#include "scene.hh"

namespace method
{
    class geometry_pass: public pipeline_method
    {
    public:
        geometry_pass(
            gbuffer& buf,
            multishader* geometry_shader,
            render_scene* scene
        );

        void set_scene(render_scene* scene);
        render_scene* get_scene() const;

        void execute() override;

    private:
        multishader* geometry_shader;
        render_scene* scene;
    };
};
#endif
