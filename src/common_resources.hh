#ifndef COMMON_RESOURCES_HH
#define COMMON_RESOURCES_HH
#include <glm/glm.hpp>
#include "resource_pool.hh"

// These make sure that the pool contains the requested common resource, such
// as many kinds of lookup textures, vertex buffers and samplers.
namespace common
{
    // Adds samplers "common_framebuffer"
    const sampler& ensure_framebuffer_sampler(sampler_pool& pool);

    // Adds sampler "common_depth"
    const sampler& ensure_depth_sampler(sampler_pool& pool);

    // Adds vertex buffer "common_quad", a square buffer with coordinates at
    // (1,1), (1,-1), (-1,1) and (-1,-1).
    const vertex_buffer& ensure_quad_vertex_buffer(vertex_buffer_pool& pool);

    // Adds texture "circular_random_<size.x>x<size.y>", a texture with
    // random 2d unit vectors.
    const texture& ensure_circular_random_texture(
        texture_pool& pool,
        glm::uvec2 size
    );

    // Adds texture "circular_poisson_<size>", a texture with
    // poisson-distributed 2d points.
    const texture& ensure_circular_poisson_texture(
        texture_pool& pool,
        unsigned size
    );
}

#endif
