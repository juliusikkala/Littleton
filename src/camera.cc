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
#include "camera.hh"
#include <limits>

namespace
{

const glm::mat4 face_rotations[6] = {
    glm::lookAt(glm::vec3(0), glm::vec3(1,0,0), glm::vec3(0,-1,0)),
    glm::lookAt(glm::vec3(0), glm::vec3(-1,0,0), glm::vec3(0,-1,0)),
    glm::lookAt(glm::vec3(0), glm::vec3(0,1,0), glm::vec3(0,0,1)),
    glm::lookAt(glm::vec3(0), glm::vec3(0,-1,0), glm::vec3(0,0,-1)),
    glm::lookAt(glm::vec3(0), glm::vec3(0,0,1), glm::vec3(0,-1,0)),
    glm::lookAt(glm::vec3(0), glm::vec3(0,0,-1), glm::vec3(0,-1,0))
};

}

namespace lt
{

camera::camera(transformable_node* parent): transformable_node(parent) {}
camera::~camera() {}

void camera::perspective(float fov, float aspect, float near)
{
    fov = glm::radians(fov);
    projection = glm::infinitePerspective(fov, aspect, near);
    clip_info = vec3(near, -1, 1);
    projection_info = vec2(2*tan(fov/2.0f)*aspect, 2*tan(fov/2.0f));
    tan_fov = tan(fov * 0.5f);
    inverse_cos_fov = vec2(
        1.0f/cos(fov * 0.5f),
        1.0f/cos(atan(tan_fov * aspect))
    );
    this->near = near;
    this->far = INFINITY;
    this->aspect = aspect;
}

void camera::perspective(float fov, float aspect, float near, float far)
{
    fov = glm::radians(fov);
    projection = glm::perspective(fov, aspect, near, far);
    clip_info = vec3(near * far, near - far, near + far);
    projection_info = vec2(2*tan(fov/2.0f)*aspect, 2*tan(fov/2.0f));
    tan_fov = tan(fov * 0.5f);
    inverse_cos_fov = vec2(
        1.0f/cos(atan(tan_fov * aspect)),
        1.0f/cos(fov * 0.5f)
    );
    this->near = near;
    this->far = far;
    this->aspect = aspect;
}

void camera::cube_perspective(float near, float far)
{
    perspective(90.0f, 1.0f, near, far);
}

mat4 camera::get_projection() const
{
    return projection;
}

vec3 camera::get_clip_info() const
{
    return clip_info;
}

vec2 camera::get_projection_info() const
{
    return projection_info;
}

vec3 camera::get_view_direction() const
{
    return get_global_orientation() * vec3(0, 0, -1);
}

vec3 camera::get_up_direction() const
{
    return get_global_orientation() * vec3(0, 1, 0);
}

mat4 camera::get_view_projection(unsigned face) const
{
    mat4 transform = glm::inverse(get_global_transform());
    return projection * face_rotations[face] * transform;
}

mat4 camera::get_inverse_orientation_projection(unsigned face) const
{
    return glm::toMat4(get_global_orientation()) *
        glm::inverse(projection * face_rotations[face]);
}

float camera::get_near() const
{
    return near;
}

float camera::get_far() const
{
    return far;
}

float camera::get_aspect() const
{
    return aspect;
}

// Using method described in http://www.lighthouse3d.com/tutorials/view-frustum-culling/radar-approach-testing-points/
bool camera::sphere_is_visible(vec3 pos, float r) const
{
    vec3 view = pos;
    vec3 pc(view.x, view.y, -view.z);

    if(pc.z > far + r || pc.z < near - r) return false;
    
    float d = r * inverse_cos_fov.y;
    pc.z *= tan_fov;
    if(pc.y > pc.z + d || pc.y < -pc.z - d) return false;

    pc.z *= aspect;
    d = r * inverse_cos_fov.x;
    if(pc.x > pc.z + d || pc.x < -pc.z - d) return false;

    return true;
}

vec2 camera::pixels_per_unit(uvec2 target_size) const
{
    vec3 top(1,1,-1);
    vec3 bottom(-1,-1,-1);
    vec4 top_projection = projection * vec4(top, 1);
    top_projection /= top_projection.w;
    vec4 bottom_projection = projection * vec4(bottom, 1);
    bottom_projection /= bottom_projection.w;

    return 0.5f * vec2(
        top_projection.x - bottom_projection.x,
        top_projection.y - bottom_projection.y
    ) * vec2(target_size);
}

} // namespace lt
