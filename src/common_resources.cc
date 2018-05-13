#include "common_resources.hh"
#include "texture.hh"
#include "glheaders.hh"
#include "math.hh"

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

static const GLfloat quad_vertices[] = {
    1.0f, 1.0f, 0, 1.0f, 1.0f,
    1.0f, -1.0f, 0, 1.0f, 0.0f,
    -1.0f, 1.0f, 0, 0.0f, 1.0f,
    -1.0f, -1.0f, 0, 0.0f, 0.0f
};

static const GLuint quad_indices[] = {0,2,1,1,2,3};

const primitive& common::ensure_quad_primitive(
    primitive_pool& prim_pool,
    gpu_buffer_pool& buf_pool
){
    std::string name = "common_quad";
    if(prim_pool.contains(name)) return *prim_pool.get(name);

    std::string indices_name = "common_quad_index_buf";
    std::string vertices_name = "common_quad_vertex_buf";

    const gpu_buffer* indices = nullptr;
    const gpu_buffer* vertices = nullptr;
    if(buf_pool.contains(indices_name)) indices = buf_pool.get(indices_name);
    else
    {
        indices = buf_pool.add(
            indices_name,
            new gpu_buffer(
                buf_pool.get_context(),
                GL_ELEMENT_ARRAY_BUFFER,
                sizeof(quad_indices),
                quad_indices
            )
        );
    }
    if(buf_pool.contains(vertices_name)) vertices = buf_pool.get(vertices_name);
    else
    {
        vertices = buf_pool.add(
            vertices_name,
            new gpu_buffer(
                buf_pool.get_context(),
                GL_ARRAY_BUFFER,
                sizeof(quad_vertices),
                quad_vertices
            )
        );
    }

    gpu_buffer_accessor index_accessor(
        *indices, 1, GL_UNSIGNED_INT, false, 0, 0
    );
    gpu_buffer_accessor position_accessor(
        *vertices, 3, GL_FLOAT, false, 5*sizeof(float), 0
    );
    gpu_buffer_accessor texture_accessor(
        *vertices, 2, GL_FLOAT, false, 5*sizeof(float), 3*sizeof(float)
    );

    return *prim_pool.add(
        name,
        new primitive(
            prim_pool.get_context(),
            sizeof(quad_indices)/sizeof(GLuint),
            GL_TRIANGLES,
            index_accessor,
            {{primitive::POSITION, position_accessor},
             {primitive::UV0, texture_accessor}}
        )
    );
}

const primitive& common::ensure_quad_primitive(resource_pool& pool)
{
    return ensure_quad_primitive(pool, pool);
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
