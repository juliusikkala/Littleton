#include "shadow_map.hh"
#include "helpers.hh"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

directional_shadow_map::directional_shadow_map(
    glm::vec3 offset,
    glm::vec2 area,
    glm::vec2 depth_range,
    directional_light* light
): up(0,1,0), l(light)
{
    set_volume(area, depth_range);
    target.set_position(offset);
}

directional_shadow_map::directional_shadow_map(
    const directional_shadow_map& other
): target(other.target), up(other.up), projection(other.projection), l(other.l)
{}

void directional_shadow_map::set_parent(transformable_node* parent)
{
    target.set_parent(parent);
}

void directional_shadow_map::set_offset(glm::vec3 offset)
{
    target.set_position(offset);
}

glm::vec3 directional_shadow_map::get_offset() const
{
    return target.get_position();
}

void directional_shadow_map::set_volume(
    glm::vec2 area,
    glm::vec2 depth_range
){
    projection = glm::ortho(
        -area.x/2,
        area.x/2,
        -area.y/2,
        area.y/2,
        depth_range.x,
        depth_range.y
    );
}

void directional_shadow_map::set_up_axis(glm::vec3 up)
{
    this->up = up;
}

glm::mat4 directional_shadow_map::get_view() const
{
    if(!l) return glm::mat4(0);

    glm::mat4 translation = glm::translate(-target.get_global_position());
    glm::mat4 rotation = glm::mat4(
        glm::inverse(quat_lookat(l->get_direction(), up))
    );
    return rotation * translation;
}

glm::mat4 directional_shadow_map::get_projection() const
{
    return projection;
}

void directional_shadow_map::set_light(directional_light* light)
{
    this->l = light;
}

directional_light* directional_shadow_map::get_light() const
{
    return l;
}

point_shadow_map::point_shadow_map(
    glm::vec2 depth_range,
    point_light* light
): l(light)
{
    set_range(depth_range);
}

point_shadow_map::point_shadow_map(const point_shadow_map& other)
: projection(other.projection), l(other.l) { }

void point_shadow_map::set_light(point_light* light)
{
    this->l = light;
}

point_light* point_shadow_map::get_light() const
{
    return l;
}

void point_shadow_map::set_range(glm::vec2 depth_range)
{
    projection = glm::perspective(
        float(M_PI/2.0f),
        1.0f,
        depth_range.x,
        depth_range.y
    );
}

glm::mat4 point_shadow_map::get_view(unsigned face) const
{
    if(!l) return glm::mat4(0);

    static const glm::mat4 face_rotations[6] = {
        glm::lookAt(glm::vec3(0), glm::vec3(1,0,0), glm::vec3(0,-1,0)),
        glm::lookAt(glm::vec3(0), glm::vec3(-1,0,0), glm::vec3(0,-1,0)),
        glm::lookAt(glm::vec3(0), glm::vec3(0,1,0), glm::vec3(0,0,1)),
        glm::lookAt(glm::vec3(0), glm::vec3(0,-1,0), glm::vec3(0,0,1)),
        glm::lookAt(glm::vec3(0), glm::vec3(0,0,1), glm::vec3(0,-1,0)),
        glm::lookAt(glm::vec3(0), glm::vec3(0,0,-1), glm::vec3(0,-1,0))
    };
    glm::mat4 transform = glm::inverse(l->get_global_transform());

    return face_rotations[face] * transform;
}

glm::mat4 point_shadow_map::get_projection() const
{
    return projection;
}
