#ifndef LT_SHADOW_MAP_HH
#define LT_SHADOW_MAP_HH
#include "light.hh"
#include "resource.hh"
#include "shader.hh"
#include <set>

namespace lt
{

namespace method
{
    class shadow_method;
}

class directional_shadow_map
{
public:
    directional_shadow_map(
        method::shadow_method* method,
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

    method::shadow_method* get_method() const;

private:
    method::shadow_method* method;
    transformable_node target;
    glm::vec3 up;
    glm::mat4 projection;

    directional_light* l;
};

class omni_shadow_map
{
public:
    omni_shadow_map(
        method::shadow_method* method,
        glm::vec2 depth_range = glm::vec2(0.01f, 10.0f),
        point_light* light = nullptr
    );
    omni_shadow_map(const omni_shadow_map& other);

    void set_light(point_light* light = nullptr);
    point_light* get_light() const;

    void set_range(glm::vec2 depth_range);
    glm::vec2 get_range() const;

    glm::mat4 get_view(unsigned face) const;
    glm::mat4 get_projection() const;

    method::shadow_method* get_method() const;

private:
    method::shadow_method* method;
    glm::vec2 range;
    glm::mat4 projection;

    point_light* l;
};

class perspective_shadow_map
{
public:
    perspective_shadow_map(
        method::shadow_method* method,
        double fov = 30,
        glm::vec2 depth_range = glm::vec2(0.01f, 10.0f),
        point_light* light = nullptr
    );
    perspective_shadow_map(const perspective_shadow_map& other);

    void set_light(point_light* light = nullptr);
    point_light* get_light() const;

    void set_range(glm::vec2 depth_range);
    glm::vec2 get_range() const;

    void set_fov(float fov);
    float get_fov() const;

    glm::mat4 get_view() const;
    glm::mat4 get_projection() const;

    method::shadow_method* get_method() const;

private:
    method::shadow_method* method;
    float fov;
    glm::vec2 range;
    glm::mat4 projection;

    point_light* l;
};

} // namespace lt
#endif
