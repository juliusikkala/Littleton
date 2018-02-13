#ifndef METHOD_SKY_HH
#define METHOD_SKY_HH
#include "pipeline.hh"
#include "shader_store.hh"
#include "vertex_buffer.hh"
#include "scene.hh"

namespace method
{
    class sky: public target_method
    {
    public:
        sky(
            render_target& target,
            shader_store& shaders,
            render_scene* scene = nullptr
        );

        void set_scene(render_scene* s);
        render_scene* get_scene() const;

        void execute() override;

    private:
        shader* sky_shader;
        render_scene* scene;
        vertex_buffer fullscreen_quad;
    };
}
#endif
