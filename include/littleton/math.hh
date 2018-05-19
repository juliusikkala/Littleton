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
#ifndef LT_MATH_HH
#define LT_MATH_HH
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/integer.hpp>
#include <cmath>
#include <vector>

#undef M_PI
#define M_PI 3.14159265359

namespace lt
{

// Add  stuff directly to lt namespace
using namespace glm;

void decompose_matrix(
    const mat4& transform,
    vec3& translation,
    vec3& scaling,
    quat& orientation
);

vec3 get_matrix_translation(const mat4& transform);
vec3 get_matrix_scaling(const mat4& transform);
quat get_matrix_orientation(const mat4& transform);

void decompose_perspective(
    const mat4& perspective,
    float& near,
    float& far,
    float& fov,
    float& aspect
);

quat rotate_towards(
    quat orig,
    quat dest,
    float angle_limit
);

quat quat_lookat(
    vec3 dir,
    vec3 up,
    vec3 forward = vec3(0,0,-1)
);

bool solve_quadratic(float a, float b, float c, float& x0, float& x1);

bool intersect_sphere(
    vec3 pos,
    vec3 dir,
    vec3 origin,
    float radius,
    float& t0,
    float& t1
);

unsigned next_power_of_two(unsigned n);
unsigned factorize(unsigned n);

// Computes a modelview matrix for a quad such that it completely covers the
// surface area of a sphere. use_near_radius determines whether the resulting
// depth value is picked from the near edge of the sphere or the furthest edge.
mat4 sphere_projection_quad_matrix(
    vec3 pos,
    float r,
    float near,
    float far,
    bool use_near_radius = false,
    float big = 1e3
);

// Note that this function is very slow. Please save your generated samples.
// If samples already contain some values, they're assumed to be generated by
// a previous call to this function with the same r.
// Circular version
void mitchell_best_candidate(
    std::vector<vec2>& samples,
    float r,
    unsigned candidate_count,
    unsigned count
);

// Rectangular version
void mitchell_best_candidate(
    std::vector<vec2>& samples,
    float w,
    float h,
    unsigned candidate_count,
    unsigned count
);

// Spherical version
void mitchell_best_candidate(
    std::vector<vec3>& samples,
    float r,
    unsigned candidate_count,
    unsigned count
);

std::vector<vec2> grid_samples(
    unsigned w,
    unsigned h,
    float step
);

std::vector<vec2> poisson_samples(
    float w,
    float h,
    float mindist
);

std::vector<float> generate_gaussian_kernel(
    int radius,
    float sigma
);


unsigned calculate_mipmap_count(uvec2 size);

} // namespace lt

#endif
