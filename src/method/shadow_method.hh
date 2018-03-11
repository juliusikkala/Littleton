#ifndef METHOD_SHADOW_METHOD_HH
#define METHOD_SHADOW_METHOD_HH
#include "pipeline.hh"
#include "render_target.hh"
#include "shadow_map.hh"

class directional_light;
class render_scene;

namespace method
{
    class shadow_method: public pipeline_method
    {
    public:
        shadow_method(render_scene* scene = nullptr);

        void set_scene(render_scene* scene);
        render_scene* get_scene() const;

        // Sets the uniforms needed when using directional shadow maps with this
        // method.
        virtual void set_directional_uniforms(
            shader* s,
            unsigned& texture_index
        );

        // Definitions needed when using the shadow maps.
        virtual shader::definition_map get_directional_definitions() const = 0;

        virtual size_t get_directional_shadow_map_count() const = 0;

        virtual directional_shadow_map*
        get_directional_shadow_map(unsigned i) const = 0;

        // Sets shadow map uniforms.
        virtual void set_directional_shadow_map_uniforms(
            shader* s,
            unsigned& texture_index,
            unsigned i,
            const std::string& prefix,
            const glm::mat4& pos_to_world
        ) = 0;
    protected:
        render_scene* scene;
    };
};
#endif
