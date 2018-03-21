#ifndef METHOD_FORWARD_PASS_HH
#define METHOD_FORWARD_PASS_HH
#include "pipeline.hh"

class render_scene;
class shader_pool;
class multishader;

namespace method
{
    class shadow_method;
    class forward_pass: public target_method
    {
    public:
        forward_pass(
            render_target& target,
            shader_pool& shaders,
            render_scene* scene,
            bool apply_ambient = true
        );
        ~forward_pass();

        void set_scene(render_scene* s);
        render_scene* get_scene() const;

        void set_apply_ambient(bool apply_ambient);
        bool get_apply_ambient() const;

        void execute() override;

        std::string get_name() const override;

    private:
        multishader* forward_shader;
        multishader* depth_shader;
        render_scene* scene; 

        bool apply_ambient;
    };
}

#endif
