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
        glm::vec3 position,
        directional_light* light = nullptr
    );
    directional_shadow_map(directional_shadow_map&& other);
    ~directional_shadow_map();

    void set_position(glm::vec3 position);
    glm::vec3 get_position() const;

    // Only works if 'light' is not nullptr
    void set_target(glm::vec3 target_pos, float distance);

    void set_light(directional_light* light = nullptr);
    directional_light* get_light() const;

    texture& get_depth();
    const texture& get_depth() const;

private:
    glm::vec3 position;
    directional_light* light;
    texture depth;
};

#endif
