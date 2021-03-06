/*
    Copyright 2018-2019 Julius Ikkala

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
#ifndef LT_CAMERA_HH
#define LT_CAMERA_HH
#include "api.hh"
#include "transformable.hh"
#include "math.hh"

namespace lt
{

class LT_API camera: public transformable_node
{
public:
    camera(transformable_node* parent = nullptr);
    ~camera();

    void perspective(float fov, float aspect, float near);
    void perspective(float fov, float aspect, float near, float far);
    void cube_perspective(float near, float far);
    mat4 get_projection() const;
    vec3 get_clip_info() const;
    vec2 get_projection_info() const;

    vec3 get_view_direction() const;
    vec3 get_up_direction() const;

    // Assumes the camera was set using cube_perspective
    mat4 get_view_projection(unsigned face) const;

    // Assumes the camera was set using cube_perspective
    mat4 get_inverse_orientation_projection(unsigned face) const;

    float get_near() const;
    float get_far() const;
    float get_aspect() const;

    // pos must be in view space
    bool sphere_is_visible(vec3 pos, float r) const;

    vec2 pixels_per_unit(uvec2 target_size) const;

private:
    mat4 projection;
    vec3 clip_info;
    vec2 projection_info;

    float near, far, aspect;
    float tan_fov;
    vec2 inverse_cos_fov;
};

} // namespace lt

#endif
