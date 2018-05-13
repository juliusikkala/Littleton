#ifndef LT_METHOD_SHADOW_METHOD_HH
#define LT_METHOD_SHADOW_METHOD_HH
#include "pipeline.hh"
#include "render_target.hh"
#include "shadow_map.hh"

namespace lt
{

class directional_light;
class render_scene;

}

namespace lt::method
{

class shadow_method: public pipeline_method
{
public:
    shadow_method(render_scene* scene = nullptr);

    void set_scene(render_scene* scene);
    render_scene* get_scene() const;

    // Sets the uniforms needed when using directional shadow maps with
    // this method.
    virtual void set_directional_uniforms(
        shader* s,
        unsigned& texture_index
    );

    // Sets the uniforms needed when using omnidirectional shadow maps with
    // this method.
    virtual void set_omni_uniforms(shader* s, unsigned& texture_index);

    // Sets the uniforms needed when using perspective shadow maps with
    // this method.
    virtual void set_perspective_uniforms(
        shader* s,
        unsigned& texture_index
    );

    // Definitions needed when using the shadow maps.
    virtual shader::definition_map get_directional_definitions() const;
    virtual shader::definition_map get_omni_definitions() const;
    virtual shader::definition_map get_perspective_definitions() const;

    // Sets shadow map uniforms. The given shadow map is assumed to be a  type
    // compatible with the method.
    virtual void set_shadow_map_uniforms(
        shader* s,
        unsigned& texture_index,
        directional_shadow_map* shadow_map,
        const std::string& prefix,
        const glm::mat4& pos_to_world
    );

    virtual void set_shadow_map_uniforms(
        shader* s,
        unsigned& texture_index,
        omni_shadow_map* shadow_map,
        const std::string& prefix,
        const glm::mat4& pos_to_world
    );

    virtual void set_shadow_map_uniforms(
        shader* s,
        unsigned& texture_index,
        perspective_shadow_map* shadow_map,
        const std::string& prefix,
        const glm::mat4& pos_to_world
    );

protected:
    render_scene* scene;
};

} // namespace lt::method

#endif
