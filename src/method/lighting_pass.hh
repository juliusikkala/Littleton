#ifndef METHOD_LIGHTING_PASS_HH
#define METHOD_LIGHTING_PASS_HH
#include "pipeline.hh"
#include "gbuffer.hh"
#include "shader_store.hh"
#include "vertex_buffer.hh"
#include "scene.hh"

namespace method
{
    class lighting_pass: public pipeline_method
    {
    public:
        lighting_pass(
            render_target& target,
            gbuffer& buf,
            shader_store& store,
            render_scene* scene
        );

        void set_scene(render_scene* scene);
        render_scene* get_scene() const;

        void execute() override;

    private:
        gbuffer* buf;

        multishader* lighting_shader;
        render_scene* scene;

        vertex_buffer fullscreen_quad;
    };
}

#endif
