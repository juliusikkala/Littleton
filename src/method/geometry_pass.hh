#ifndef METHOD_GEOMETRY_PASS_HH
#define METHOD_GEOMETRY_PASS_HH
#include "pipeline.hh"

class gbuffer;
class shader_pool;
class render_scene;
class multishader;

namespace method
{
    class geometry_pass: public target_method
    {
    public:
        geometry_pass(
            gbuffer& buf,
            shader_pool& store,
            render_scene* scene
        );

        void set_scene(render_scene* scene);
        render_scene* get_scene() const;

        void execute() override;

        std::string get_name() const override;

    private:
        multishader* geometry_shader;
        render_scene* scene;
    };
};
#endif
