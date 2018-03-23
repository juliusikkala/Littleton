#include "common_resources.hh"
#include "texture.hh"
#include "glheaders.hh"
#include "helpers.hh"
#include <glm/gtc/random.hpp>

static texture* generate_circular_random(context& ctx, glm::uvec2 size)
{
    std::vector<glm::vec2> circular_samples;
    circular_samples.resize(size.x * size.y);

    for(unsigned i = 0; i < size.x * size.y; ++i)
    {
        circular_samples[i] = glm::circularRand(1.0f);
    }

    return new texture(
        ctx,
        size,
        GL_RG8_SNORM,
        GL_FLOAT,
        0,
        GL_TEXTURE_2D,
        (float*)circular_samples.data()
    );
}

static texture* generate_spherical_random(context& ctx, glm::uvec2 size)
{
    std::vector<glm::vec3> spherical_samples;
    spherical_samples.resize(size.x * size.y);

    for(unsigned i = 0; i < size.x * size.y; ++i)
    {
        spherical_samples[i] = glm::sphericalRand(1.0f);
    }

    return new texture(
        ctx,
        size,
        GL_RGB8_SNORM,
        GL_FLOAT,
        0,
        GL_TEXTURE_2D,
        (float*)spherical_samples.data()
    );
}

static texture* generate_circular_poisson(context& ctx, unsigned size)
{
    std::vector<glm::vec2> poisson;

    mitchell_best_candidate(
        poisson,
        1.0f,
        20,
        size
    );

    return new texture(
        ctx,
        glm::uvec2(size),
        GL_RG8_SNORM,
        GL_FLOAT,
        0,
        GL_TEXTURE_1D,
        (float*)poisson.data()
    );
}

static texture* generate_spherical_poisson(context& ctx, unsigned size)
{
    std::vector<glm::vec3> poisson;

    mitchell_best_candidate(
        poisson,
        1.0f,
        20,
        size
    );

    return new texture(
        ctx,
        glm::uvec2(size),
        GL_RGB8_SNORM,
        GL_FLOAT,
        0,
        GL_TEXTURE_1D,
        (float*)poisson.data()
    );
}

static const GLfloat quad_vertices[] = {
    1.0f, 1.0f, 0, 1.0f, 1.0f,
    1.0f, -1.0f, 0, 1.0f, 0.0f,
    -1.0f, 1.0f, 0, 0.0f, 1.0f,
    -1.0f, -1.0f, 0, 0.0f, 0.0f
};

static const GLuint quad_indices[] = {0,2,1,1,2,3};

static vertex_buffer* create_quad(context& ctx)
{
    return new vertex_buffer(
        ctx,
        vertex_buffer::VERTEX_PT,
        sizeof(quad_vertices)/(sizeof(float)*5),
        quad_vertices,
        sizeof(quad_indices)/sizeof(GLuint),
        quad_indices
    );
}

const sampler& common::ensure_framebuffer_sampler(sampler_pool& pool)
{
    std::string name = "common_framebuffer";
    if(pool.contains(name)) return *pool.get(name);
    return *pool.add(name,
        new sampler(pool.get_context(),
            GL_NEAREST, GL_NEAREST,
            GL_CLAMP_TO_EDGE
        )
    );
}

const sampler& common::ensure_depth_sampler(sampler_pool& pool)
{
    std::string name = "common_depth";
    if(pool.contains(name)) return *pool.get(name);
    return *pool.add(name,
        new sampler(pool.get_context(),
            GL_NEAREST, GL_NEAREST,
            GL_CLAMP_TO_BORDER,
            0, glm::vec4(1)
        )
    );
}

const vertex_buffer& common::ensure_quad_vertex_buffer(vertex_buffer_pool& pool)
{
    std::string name = "common_quad";
    if(pool.contains(name)) return *pool.get(name);
    return *pool.add(name, create_quad(pool.get_context()));
}

const texture& common::ensure_circular_random_texture(
    texture_pool& pool,
    glm::uvec2 size
){
    std::string name =
        "circular_random_" + std::to_string(size.x)
        + "x" + std::to_string(size.y);

    if(pool.contains(name)) return *pool.get(name);
    return *pool.add(name, generate_circular_random(pool.get_context(), size));
}

const texture& common::ensure_circular_poisson_texture(
    texture_pool& pool,
    unsigned size
){
    std::string name = "circular_poisson_" + std::to_string(size);

    if(pool.contains(name)) return *pool.get(name);
    return *pool.add(name, generate_circular_poisson(pool.get_context(), size));
}

const texture& common::ensure_spherical_random_texture(
    texture_pool& pool,
    glm::uvec2 size
){
    std::string name =
        "spherical_random_" + std::to_string(size.x)
        + "x" + std::to_string(size.y);

    if(pool.contains(name)) return *pool.get(name);
    return *pool.add(name, generate_spherical_random(pool.get_context(), size));
}

const texture& common::ensure_spherical_poisson_texture(
    texture_pool& pool,
    unsigned size
){
    std::string name = "spherical_poisson_" + std::to_string(size);

    if(pool.contains(name)) return *pool.get(name);
    return *pool.add(name, generate_spherical_poisson(pool.get_context(), size));
}
