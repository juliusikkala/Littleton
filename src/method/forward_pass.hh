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
            std::vector<shadow_method*>&& shadows = {}
        );
        ~forward_pass();

        void set_scene(render_scene* s);
        render_scene* get_scene() const;

        void execute() override;

        std::string get_name() const override;

    private:
        multishader* forward_shader;
        multishader* depth_shader;
        render_scene* scene; 
        std::vector<shadow_method*> shadows;
    };
}

#endif
