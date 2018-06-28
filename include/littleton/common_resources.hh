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
#ifndef LT_COMMON_RESOURCES_HH
#define LT_COMMON_RESOURCES_HH
#include "api.hh"
#include "resource_pool.hh"
#include "math.hh"

// These make sure that the pool contains the requested common resource, such
// as many kinds of lookup textures, vertex buffers and samplers.
namespace lt::common
{
// Adds sampler "common_linear"
LT_API const sampler& ensure_linear_sampler(sampler_pool& pool);

// Adds sampler "common_framebuffer"
LT_API const sampler& ensure_framebuffer_sampler(sampler_pool& pool);

// Adds sampler "common_depth"
LT_API const sampler& ensure_depth_sampler(sampler_pool& pool);

// Adds primitive "common_quad", a square buffer with coordinates at
// (1,1), (1,-1), (-1,1) and (-1,-1).
LT_API const primitive& ensure_quad_primitive(
    primitive_pool& prim_pool,
    gpu_buffer_pool& buf_pool
);

LT_API const primitive& ensure_quad_primitive(resource_pool& pool);

// Adds primitive "common_cube", a unit cube buffer.
LT_API const primitive& ensure_cube_primitive(
    primitive_pool& prim_pool,
    gpu_buffer_pool& buf_pool
);

LT_API const primitive& ensure_cube_primitive(resource_pool& pool);

// Adds primitive "common_patched_sphere_<subdivisions>", a unit-sized patched
// sphere (http://www.iquilezles.org/www/articles/patchedsphere/patchedsphere.htm)
LT_API const primitive& ensure_patched_sphere_primitive(
    primitive_pool& prim_pool,
    gpu_buffer_pool& buf_pool,
    unsigned subdivisions
);

LT_API const primitive& ensure_patched_sphere_primitive(
    resource_pool& pool,
    unsigned subdivisions
);

// Adds texture "circular_random_<size.x>x<size.y>", a texture with
// random 2d unit vectors.
LT_API const texture& ensure_circular_random_texture(
    texture_pool& pool,
    glm::uvec2 size
);

// Adds texture "spherical_random_<size.x>x<size.y>", a texture with
// random 3d unit vectors.
LT_API const texture& ensure_spherical_random_texture(
    texture_pool& pool,
    glm::uvec2 size
);

// Adds texture "circular_poisson_<size>", a texture with
// poisson-distributed 2d points.
LT_API const texture& ensure_circular_poisson_texture(
    texture_pool& pool,
    unsigned size
);

// Adds texture "spherical_poisson_<size>", a texture with
// poisson-distributed 2d points.
LT_API const texture& ensure_spherical_poisson_texture(
    texture_pool& pool,
    unsigned size
);

} // namespace lt::common

#endif
