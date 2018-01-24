#ifndef METHOD_FORWARD_RENDER_HH
#define METHOD_FORWARD_RENDER_HH
#include "pipeline.hh"
#include "shader_cache.hh"
#include "scene.hh"

namespace method
{
    class forward_render: public pipeline_method
    {
    public:
        forward_render(
            shader_cache* forward_shader = nullptr,
            scene* render_scene = nullptr
        );
        ~forward_render();

        void set_shader_cache(shader_cache* s);
        shader_cache* get_shader_cache() const;

        void set_scene(scene* s);
        scene* get_scene() const;

        void execute() override;

    private:
        shader_cache* forward_shader;
        scene* render_scene;
    };
}

#endif
