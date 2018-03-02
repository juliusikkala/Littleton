#ifndef SHADOW_MAP_HH
#define SHADOW_MAP_HH
#include "texture.hh"
#include "light.hh"
#include "render_target.hh"
#include "shader_store.hh"

class render_scene;
class basic_shadow_map: public glresource
{
public:
    basic_shadow_map(context& ctx);
    virtual ~basic_shadow_map();

    struct shared_resources
    {
        virtual ~shared_resources();

        // Definitions needed when using the shadow map
        virtual shader::definition_map get_definitions() const = 0;
 
        // Must set the uniforms needed when using the shadow map
        virtual void set_uniforms(
            shader* s,
            // First available texture index.
            // Please update this when using an index.
            unsigned& texture_index
        ) = 0;
    };

    // Should render the shadow map itself.
    virtual void render(
        shader_store& store,
        render_scene* scene,
        shared_resources* resources
    ) = 0;

    // Return false if the given resources can't be upgraded to be compatible
    // with the ones required by this shadow map.
    virtual bool merge_shared_resources(shared_resources* res) const = 0;

    // Must create the minimum-required resources for this shadow map.
    virtual shared_resources* create_shared_resources() const = 0;

    virtual void set_uniforms(
        shader* s,
        const std::string& prefix,
        // First available texture index.
        // Please update this when using an index.
        unsigned& texture_index,
        const glm::mat4& pos_to_world
    ) = 0;

    virtual light* get_light() const = 0;
};

class directional_shadow_map: public basic_shadow_map
{
public:
    directional_shadow_map(
        context& ctx,
        glm::vec3 offset = glm::vec3(0),
        glm::vec2 area = glm::vec2(1.0f),
        glm::vec2 depth_range = glm::vec2(1.0f, -1.0f),
        directional_light* light = nullptr
    );
    directional_shadow_map(const directional_shadow_map& other);

    void set_parent(transformable_node* parent);

    void set_light(directional_light* light = nullptr);
    directional_light* get_light() const override;

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

    directional_light* light;
};

texture* generate_shadow_noise_texture(context& ctx, glm::uvec2 size);

#endif
