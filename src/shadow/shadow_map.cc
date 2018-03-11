#include "shadow_map.hh"
#include "helpers.hh"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

basic_shadow_map::basic_shadow_map(context& ctx)
: glresource(ctx) {}

basic_shadow_map::~basic_shadow_map() {}

directional_shadow_map::directional_shadow_map(
    context& ctx,
    glm::vec3 offset,
    glm::vec2 area,
    glm::vec2 depth_range,
    directional_light* light
): basic_shadow_map(ctx), up(0,1,0), l(light)
{
    set_volume(area, depth_range);
    target.set_position(offset);
}

directional_shadow_map::directional_shadow_map(
    const directional_shadow_map& other
):  basic_shadow_map(other),
    target(other.target), up(other.up), projection(other.projection), l(other.l)
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
