#ifndef METHOD_FORWARD_PASS_HH
#define METHOD_FORWARD_PASS_HH
#include "pipeline.hh"
#include "multishader.hh"
#include "scene.hh"

namespace method
{
    class forward_pass: public pipeline_method
    {
    public:
        forward_pass(
            render_target& target,
            multishader* forward_shader = nullptr,
            render_scene* scene = nullptr
        );
        ~forward_pass();

        void set_shader(multishader* s);
        multishader* get_shader() const;

        void set_scene(render_scene* s);
        render_scene* get_scene() const;

        void execute() override;

    private:
        multishader* forward_shader;
        render_scene* scene;
    };
}

#endif
