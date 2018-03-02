#ifndef METHOD_RENDER_SHADOW_MAPS_HH
#define METHOD_RENDER_SHADOW_MAPS_HH
#include "pipeline.hh"
#include "shader_store.hh"
#include "scene.hh"

namespace method
{
    class render_shadow_maps: public pipeline_method
    {
    public:
        render_shadow_maps(
            shader_store& shaders,
            render_scene* scene = nullptr
        );

        void set_scene(render_scene* s);
        render_scene* get_scene() const;

        void execute() override;

    private:
        shader_store* store;
        render_scene* scene;
    };
}
#endif
