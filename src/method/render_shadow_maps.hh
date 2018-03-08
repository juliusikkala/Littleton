#ifndef METHOD_RENDER_SHADOW_MAPS_HH
#define METHOD_RENDER_SHADOW_MAPS_HH
#include "pipeline.hh"

class resource_pool;
class render_scene;

namespace method
{
    class render_shadow_maps: public pipeline_method
    {
    public:
        render_shadow_maps(
            resource_pool& pool,
            render_scene* scene = nullptr
        );

        void set_scene(render_scene* s);
        render_scene* get_scene() const;

        void execute() override;

        std::string get_name() const override;

    private:
        resource_pool* pool;
        render_scene* scene;
    };
}
#endif
