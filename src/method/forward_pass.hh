#ifndef METHOD_FORWARD_PASS_HH
#define METHOD_FORWARD_PASS_HH
#include "pipeline.hh"
#include "shader_cache.hh"
#include "scene.hh"

namespace method
{
    class forward_pass: public pipeline_method
    {
    public:
        forward_pass(
            render_target& target,
            shader_cache* forward_shader = nullptr,
            render_scene* scene = nullptr
        );
        ~forward_pass();

        void set_shader_cache(shader_cache* s);
        shader_cache* get_shader_cache() const;

        void set_scene(render_scene* s);
        render_scene* get_scene() const;

        void execute() override;

    private:
        shader_cache* forward_shader;
        render_scene* scene;
    };
}

#endif
