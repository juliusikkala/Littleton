/*
    Copyright 2018 Julius Ikkala

    This file is part of Littleton.

    Littleton is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Littleton is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Littleton.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "shadow_map.hh"
#include "helpers.hh"
#include "math.hh"

namespace lt
{

directional_shadow_map::directional_shadow_map(
    method::shadow_method* method,
    glm::vec3 offset,
    glm::vec2 area,
    glm::vec2 depth_range,
    directional_light* light
): method(method), up(0,1,0), l(light)
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

method::shadow_method* directional_shadow_map::get_method() const
{
    return method;
}

void directional_shadow_map::set_light(directional_light* light)
{
    this->l = light;
}

directional_light* directional_shadow_map::get_light() const
{
    return l;
}

omni_shadow_map::omni_shadow_map(
    method::shadow_method* method,
    glm::vec2 depth_range,
    point_light* light
): method(method), l(light)
{
    set_range(depth_range);
}

omni_shadow_map::omni_shadow_map(const omni_shadow_map& other)
: projection(other.projection), l(other.l) { }

void omni_shadow_map::set_light(point_light* light)
{
    this->l = light;
}

point_light* omni_shadow_map::get_light() const
{
    return l;
}

void omni_shadow_map::set_range(glm::vec2 depth_range)
{
    this->range = depth_range;
    projection = glm::perspective(
        float(M_PI/2.0f),
        1.0f,
        depth_range.x,
        depth_range.y
    );
}

glm::vec2 omni_shadow_map::get_range() const
{
    return range;
}

glm::mat4 omni_shadow_map::get_view(unsigned face) const
{
    if(!l) return glm::mat4(0);

    static const glm::mat4 face_rotations[6] = {
        glm::lookAt(glm::vec3(0), glm::vec3(1,0,0), glm::vec3(0,-1,0)),
        glm::lookAt(glm::vec3(0), glm::vec3(-1,0,0), glm::vec3(0,-1,0)),
        glm::lookAt(glm::vec3(0), glm::vec3(0,1,0), glm::vec3(0,0,1)),
        glm::lookAt(glm::vec3(0), glm::vec3(0,-1,0), glm::vec3(0,0,-1)),
        glm::lookAt(glm::vec3(0), glm::vec3(0,0,1), glm::vec3(0,-1,0)),
        glm::lookAt(glm::vec3(0), glm::vec3(0,0,-1), glm::vec3(0,-1,0))
    };
    glm::mat4 transform = glm::inverse(l->get_global_transform());

    return face_rotations[face] * transform;
}

glm::mat4 omni_shadow_map::get_projection() const
{
    return projection;
}

method::shadow_method* omni_shadow_map::get_method() const
{
    return method;
}

perspective_shadow_map::perspective_shadow_map(
    method::shadow_method* method,
    double fov,
    glm::vec2 depth_range,
    point_light* light
): method(method), l(light)
{
    set_fov((float)fov);
    set_range(depth_range);
}

perspective_shadow_map::perspective_shadow_map(
    const perspective_shadow_map& other
):  method(other.method), fov(other.fov), range(other.range),
    projection(other.projection), l(other.l)
{
}

void perspective_shadow_map::set_light(point_light* light)
{
    l = light;
}

point_light* perspective_shadow_map::get_light() const
{
    return l;
}

void perspective_shadow_map::set_range(glm::vec2 depth_range)
{
    this->range = depth_range;
    projection = glm::perspective(glm::radians(fov), 1.0f, range.x, range.y);
}

glm::vec2 perspective_shadow_map::get_range() const
{
    return range;
}

void perspective_shadow_map::set_fov(float fov)
{
    this->fov = fov;
    projection = glm::perspective(glm::radians(fov), 1.0f, range.x, range.y);
}

float perspective_shadow_map::get_fov() const
{
    return fov;
}

glm::mat4 perspective_shadow_map::get_view() const
{
    if(!l) return glm::mat4(0);

    glm::mat4 translation = glm::translate(-l->get_global_position());
    glm::mat4 rotation = glm::toMat4(glm::inverse(l->get_global_orientation()));
    return rotation * translation;
}

glm::mat4 perspective_shadow_map::get_projection() const
{
    return projection;
}

method::shadow_method* perspective_shadow_map::get_method() const
{
    return method;
}

} // namespace lt

