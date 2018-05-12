#ifndef METHOD_VISUALIZE_GBUFFER_HH
#define METHOD_VISUALIZE_GBUFFER_HH
#include "pipeline.hh"
#include "primitive.hh"
#include "shader.hh"
#include "sampler.hh"

class gbuffer;
class resource_pool;
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
            resource_pool& pool,
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

        std::string get_name() const override;

    private:
        gbuffer* buf;

        multishader* visualize_shader;
        render_scene* scene;
        const primitive& quad;
        const sampler& fb_sampler;        

        std::vector<visualizer> visualizers;
    };
}
#endif
