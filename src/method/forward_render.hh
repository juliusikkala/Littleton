#ifndef METHOD_FORWARD_RENDER_HH
#define METHOD_FORWARD_RENDER_HH
#include "pipeline.hh"
#include "shader.hh"
#include "scene.hh"

namespace method
{
    class forward_render: public pipeline_method
    {
    public:
        forward_render(
            shader* forward_shader = nullptr,
            scene* render_scene = nullptr
        );
        ~forward_render();

        void set_shader(shader* s);
        shader* get_shader() const;

        void set_scene(scene* s);
        scene* get_scene() const;

        void execute() override;

    private:
        shader* forward_shader;
        scene* render_scene;
    };
}

#endif
