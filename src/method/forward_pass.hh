#ifndef METHOD_FORWARD_PASS_HH
#define METHOD_FORWARD_PASS_HH
#include "pipeline.hh"

class render_scene;
class shader_store;
class multishader;

namespace method
{
    class forward_pass: public target_method
    {
    public:
        forward_pass(
            render_target& target,
            shader_store& shaders,
            render_scene* scene = nullptr
        );
        ~forward_pass();

        void set_shader(multishader* s);
        multishader* get_shader() const;

        void set_scene(render_scene* s);
        render_scene* get_scene() const;

        void execute() override;

        std::string get_name() const override;

    private:
        multishader* forward_shader;
        render_scene* scene; 
    };
}

#endif
