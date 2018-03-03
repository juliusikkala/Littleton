#ifndef SHADOW_MAP_HH
#define SHADOW_MAP_HH
#include "texture.hh"
#include "light.hh"
#include "render_target.hh"
#include "shader_store.hh"

class basic_shadow_map;
class render_scene;
class shadow_map_impl: public glresource
{
public:
    shadow_map_impl(context& ctx);
    virtual ~shadow_map_impl();

    // Renders the shadow map itself.
    virtual void render(
        shader_store& store,
        const std::set<basic_shadow_map*>& shadow_maps,
        render_scene* scene
    ) = 0;

    // Definitions needed when using the shadow maps.
    virtual shader::definition_map get_definitions() const = 0;

    // Sets the uniforms needed when using this implementation.
    virtual void set_common_uniforms(shader* s, unsigned& texture_index) = 0;

    // Sets the uniforms needed when using the shadow map.
    virtual void set_shadow_map_uniforms(
        shader* s,
        unsigned& texture_index,
        basic_shadow_map* shadow_map,
        const std::string& prefix,
        const glm::mat4& pos_to_world
    ) = 0;
};

class basic_shadow_map: public glresource
{
public:
    basic_shadow_map(context& ctx);
    virtual ~basic_shadow_map();

    virtual bool impl_is_compatible(const shadow_map_impl* impl) = 0;
    virtual shadow_map_impl* create_impl() const = 0;

    virtual light* get_light() const = 0;
    virtual glm::mat4 get_view() const = 0;
    virtual glm::mat4 get_projection() const = 0;
};

class directional_shadow_map
{
public:
    directional_shadow_map(
        glm::vec3 offset = glm::vec3(0),
        glm::vec2 area = glm::vec2(1.0f),
        glm::vec2 depth_range = glm::vec2(1.0f, -1.0f),
        directional_light* light = nullptr
    );
    directional_shadow_map(const directional_shadow_map& other);

    void set_parent(transformable_node* parent);

    void set_light(directional_light* light = nullptr);
    directional_light* get_light() const;

    void set_offset(glm::vec3 offset);
    glm::vec3 get_offset() const;

    void set_volume(glm::vec2 area, glm::vec2 depth_range);
    void set_up_axis(glm::vec3 up = glm::vec3(0,1,0));

    glm::mat4 get_view() const;
    glm::mat4 get_projection() const;

private:
    transformable_node target;
    glm::vec3 up;
    glm::mat4 projection;

    directional_light* l;
};

#endif
