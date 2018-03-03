#ifndef METHOD_VISUALIZE_GBUFFER_HH
#define METHOD_VISUALIZE_GBUFFER_HH
#include "pipeline.hh"
#include "vertex_buffer.hh"

class gbuffer;
class shader_store;
class render_scene;
class multishader;

namespace method
{
    class visualize_gbuffer: public target_method
    {
    public:
        visualize_gbuffer(
            render_target& target,
            gbuffer& buf,
            shader_store& store,
            render_scene* scene
        );

        void set_scene(render_scene* scene);
        render_scene* get_scene() const;

        enum visualizer
        {
            DEPTH,
            POSITION,
            NORMAL,
            COLOR,
            ROUGHNESS,
            METALLIC,
            IOR,
            MATERIAL
        };

        void show(visualizer full);
        void show(
            visualizer topleft,
            visualizer topright,
            visualizer bottomleft,
            visualizer bottomright
        );

        void execute() override;

    private:
        gbuffer* buf;

        multishader* visualize_shader;
        render_scene* scene;
        vertex_buffer fullscreen_quad;

        std::vector<visualizer> visualizers;
    };
}
#endif
