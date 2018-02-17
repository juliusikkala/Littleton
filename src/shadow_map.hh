#ifndef SHADOW_MAP_HH
#define SHADOW_MAP_HH
#include "texture.hh"
#include "light.hh"
#include "render_target.hh"

class directional_shadow_map: public render_target
{
public:
    directional_shadow_map(
        context& ctx,
        glm::uvec2 size,
        glm::vec3 offset = glm::vec3(0),
        glm::vec2 area = glm::vec2(1.0f),
        glm::vec2 depth_range = glm::vec2(1.0f, -1.0f),
        directional_light* light = nullptr
    );
    directional_shadow_map(directional_shadow_map&& other);
    ~directional_shadow_map();

    void set_parent(transformable_node* parent);

    void set_offset(glm::vec3 offset);
    glm::vec3 get_offset() const;

    void set_volume(glm::vec2 area, glm::vec2 depth_range);
    void set_up_axis(glm::vec3 up = glm::vec3(0,1,0));

    glm::mat4 get_view() const;
    glm::mat4 get_projection() const;

    void set_light(directional_light* light = nullptr);
    directional_light* get_light() const;

    texture& get_depth();
    const texture& get_depth() const;

private:
    transformable_node target;
    glm::vec3 up;
    glm::mat4 projection;

    directional_light* light;
    texture depth;
};

#endif
