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
#include "common_resources.hh"
#include "texture.hh"
#include "glheaders.hh"
#include "math.hh"
#include <algorithm>

namespace
{
using namespace lt;

texture* generate_circular_random(context& ctx, glm::uvec2 size)
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

texture* generate_spherical_random(context& ctx, glm::uvec2 size)
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

texture* generate_circular_poisson(context& ctx, unsigned size)
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

texture* generate_spherical_poisson(context& ctx, unsigned size)
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

primitive* create_primitive(
    gpu_buffer_pool& buf_pool,
    const std::string& name,
    const GLfloat* vertices,
    size_t vertices_count,
    bool has_normals,
    bool has_tangents,
    const GLuint* indices,
    size_t indices_size
){
    std::string indices_name = name + "_index_buf";
    std::string vertices_name = name + "_vertex_buf";

    size_t vertex_size = 5;
    if(has_normals) vertex_size += 3;
    if(has_tangents) vertex_size += 4;
    vertex_size *= sizeof(GLfloat);

    const gpu_buffer* indices_buf= nullptr;
    const gpu_buffer* vertices_buf = nullptr;
    if(buf_pool.contains(indices_name)) indices_buf = buf_pool.get(indices_name);
    else
    {
        indices_buf = buf_pool.add(
            indices_name,
            new gpu_buffer(
                buf_pool.get_context(),
                GL_ELEMENT_ARRAY_BUFFER,
                sizeof(GLuint) * indices_size,
                indices
            )
        );
    }

    if(buf_pool.contains(vertices_name)) vertices_buf = buf_pool.get(vertices_name);
    else
    {
        vertices_buf = buf_pool.add(
            vertices_name,
            new gpu_buffer(
                buf_pool.get_context(),
                GL_ARRAY_BUFFER,
                vertices_count * vertex_size,
                vertices
            )
        );
    }

    gpu_buffer_accessor index_accessor(
        *indices_buf, 1, GL_UNSIGNED_INT, false, 0, 0
    );

    std::map<primitive::attribute, gpu_buffer_accessor> attributes;
    unsigned offset = 0;

    gpu_buffer_accessor position_accessor(
        *vertices_buf, 3, GL_FLOAT, false, vertex_size, 0
    );
    offset += 3;
    attributes[primitive::POSITION] = position_accessor;

    if(has_normals)
    {
        gpu_buffer_accessor normal_accessor(
            *vertices_buf, 3, GL_FLOAT, true, vertex_size,
            offset*sizeof(GLfloat)
        );
        offset += 3;
        attributes[primitive::NORMAL] = normal_accessor;
    }

    if(has_tangents)
    {
        gpu_buffer_accessor tangent_accessor(
            *vertices_buf, 4, GL_FLOAT, true, vertex_size,
            offset*sizeof(GLfloat)
        );
        offset += 4;
        attributes[primitive::TANGENT] = tangent_accessor;
    }

    gpu_buffer_accessor texture_accessor(
        *vertices_buf, 2, GL_FLOAT, false, vertex_size, offset*sizeof(GLfloat)
    );
    attributes[primitive::UV0] = texture_accessor;

    return new primitive(
        buf_pool.get_context(),
        indices_size,
        GL_TRIANGLES,
        index_accessor,
        attributes
    );
}

const GLfloat quad_vertices[] = {
    1.0f, 1.0f, 0, 1.0f, 1.0f,
    1.0f, -1.0f, 0, 1.0f, 0.0f,
    -1.0f, 1.0f, 0, 0.0f, 1.0f,
    -1.0f, -1.0f, 0, 0.0f, 0.0f
};

const GLfloat quad_nt_vertices[] = {
    1.0f, 1.0f, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f
};

const GLuint quad_indices[] = {0,2,1,1,2,3};

const GLfloat cube_vertices[] = {
     1, -1, -1,  0,  0, -1, 1, 0,
    -1,  1, -1,  0,  0, -1, 0, 1,
     1,  1, -1,  0,  0, -1, 0, 0,
    -1,  1,  1,  0,  0,  1, 1, 0,
     1, -1,  1,  0,  0,  1, 0, 1,
     1,  1,  1,  0,  0,  1, 0, 0,
     1,  1,  1,  1,  0,  0, 1, 0,
     1, -1, -1,  1,  0,  0, 0, 1,
     1,  1, -1,  1,  0,  0, 0, 0,
     1, -1,  1,  0, -1,  0, 1, 0,
    -1, -1, -1,  0, -1,  0, 0, 1,
     1, -1, -1,  0, -1,  0, 0, 0,
    -1, -1, -1, -1,  0,  0, 0, 0,
    -1,  1,  1, -1,  0,  0, 1, 1,
    -1,  1, -1, -1,  0,  0, 0, 1,
     1,  1, -1,  0,  1,  0, 1, 0,
    -1,  1,  1,  0,  1,  0, 0, 1,
     1,  1,  1,  0,  1,  0, 0, 0,
    -1, -1, -1,  0,  0, -1, 1, 1,
    -1, -1,  1,  0,  0,  1, 1, 1,
     1,  1,  1,  1,  0,  0, 1, 0,
     1, -1,  1,  1,  0,  0, 1, 1,
     1, -1, -1,  1,  0,  0, 0, 1,
    -1, -1,  1,  0, -1,  0, 1, 1,
    -1, -1,  1, -1,  0,  0, 1, 0,
    -1,  1, -1,  0,  1,  0, 1, 1
};

const GLuint cube_indices[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 0, 18, 1,
    3, 19, 4, 20, 21, 22, 9, 23, 10, 12, 24, 13, 15, 25, 16
};

}

namespace lt::common
{

const sampler& ensure_linear_sampler(sampler_pool& pool)
{
    std::string name = "common_linear";
    if(pool.contains(name)) return *pool.get(name);
    return *pool.add(name,
        new sampler(pool.get_context(),
            interpolation::LINEAR, interpolation::LINEAR,
            GL_CLAMP_TO_EDGE
        )
    );
}

const sampler& ensure_framebuffer_sampler(sampler_pool& pool)
{
    std::string name = "common_framebuffer";
    if(pool.contains(name)) return *pool.get(name);
    return *pool.add(name,
        new sampler(pool.get_context(),
            interpolation::NEAREST, interpolation::NEAREST,
            GL_CLAMP_TO_EDGE
        )
    );
}

const sampler& ensure_depth_sampler(sampler_pool& pool)
{
    std::string name = "common_depth";
    if(pool.contains(name)) return *pool.get(name);
    return *pool.add(name,
        new sampler(pool.get_context(),
            interpolation::NEAREST, interpolation::NEAREST,
            GL_CLAMP_TO_BORDER,
            0, glm::vec4(1)
        )
    );
}

const sampler& ensure_generic_sampler(
    sampler_pool& pool, interpolation mag, interpolation min, int lod_bias 
){
    std::string name = "common_" + to_string(mag) + "_" + to_string(min);
    if(lod_bias)
    {
        name += "_";
        name += lod_bias < 0 ? "n" : "p";
        name += std::to_string(abs(lod_bias));
    }
    if(pool.contains(name)) return *pool.get(name);

    sampler* s = new sampler(pool.get_context(), mag, min);
    if(lod_bias) s->set_lod_bias(lod_bias);
    return *pool.add(name, s);
}

const primitive& ensure_quad_primitive(
    primitive_pool& prim_pool,
    gpu_buffer_pool& buf_pool
){
    std::string name = "common_quad";
    if(prim_pool.contains(name)) return *prim_pool.get(name);

    return *prim_pool.add(
        name,
        create_primitive(
            buf_pool,
            name,
            quad_vertices,
            4,
            false,
            false,
            quad_indices,
            sizeof(quad_indices)/sizeof(quad_indices[0])
        )
    );
}

const primitive& ensure_quad_primitive(resource_pool& pool)
{
    return ensure_quad_primitive(pool, pool);
}

const primitive& ensure_quad_nt_primitive(
    primitive_pool& prim_pool,
    gpu_buffer_pool& buf_pool
){
    std::string name = "common_quad_nt";
    if(prim_pool.contains(name)) return *prim_pool.get(name);

    return *prim_pool.add(
        name,
        create_primitive(
            buf_pool,
            name,
            quad_nt_vertices,
            4,
            true,
            true,
            quad_indices,
            sizeof(quad_indices)/sizeof(quad_indices[0])
        )
    );
}

const primitive& ensure_quad_nt_primitive(resource_pool& pool)
{
    return ensure_quad_nt_primitive(pool, pool);
}

const primitive& ensure_cube_primitive(
    primitive_pool& prim_pool,
    gpu_buffer_pool& buf_pool
){
    std::string name = "common_cube";
    if(prim_pool.contains(name)) return *prim_pool.get(name);

    return *prim_pool.add(
        name,
        create_primitive(
            buf_pool,
            name,
            cube_vertices,
            sizeof(cube_vertices)/(sizeof(cube_vertices[0])*5),
            true,
            false,
            cube_indices,
            sizeof(cube_indices)/sizeof(cube_indices[0])
        )
    );
}

const primitive& ensure_cube_primitive(resource_pool& pool)
{
    return ensure_cube_primitive(pool, pool);
}

const primitive& ensure_patched_sphere_primitive(
    primitive_pool& prim_pool,
    gpu_buffer_pool& buf_pool,
    unsigned subdivisions
){
    std::string name = "common_patched_sphere_" + std::to_string(subdivisions);
    if(prim_pool.contains(name)) return *prim_pool.get(name);

    struct vertex
    {
        vec3 pos;
        vec3 normal;
        vec2 uv;
    };
    std::vector<vertex> vertices;
    std::vector<GLuint> indices;

    float start = -1.0f;
    float step = 2.0f / subdivisions;

    for(unsigned face = 0; face < 6; ++face)
    {
        // Iterate subdivided faces
        for(unsigned j = 0; j < subdivisions; ++j)
        {
            for(unsigned i = 0; i < subdivisions; ++i)
            {
                float s1 = start + step * j;
                float t1 = start + step * i;
                float s2 = s1 + step;
                float t2 = t1 + step;

                vec3 face_vertices[] = {
                    vec3(s1,t1,1.0f),
                    vec3(s1,t2,1.0f),
                    vec3(s2,t1,1.0f),
                    vec3(s2,t2,1.0f)
                };

                vec2 face_uvs[] = {
                    vec2(s1,t1),
                    vec2(s1,t2),
                    vec2(s2,t1),
                    vec2(s2,t2)
                };

                GLuint face_indices[] = {
                    2, 1, 0, 3, 1, 2
                };

                for(unsigned k = 0; k < 4; ++k)
                {
                    vec3& v = face_vertices[k];
                    v = swizzle_for_cube_face(normalize(v), face);
                }

                for(GLuint index: face_indices)
                {
                    vec3 p = face_vertices[index];
                    auto it = std::find_if(
                        vertices.begin(),
                        vertices.end(),
                        [p](vertex& v){ return dot(v.pos, p) >= 0.9999f; }
                    );
                    indices.push_back(std::distance(vertices.begin(), it));
                    if(it == vertices.end())
                    {
                        vertices.push_back({
                            p, p, face_uvs[index]
                        });
                    }
                }
            }
        }
    }

    return *prim_pool.add(
        name,
        create_primitive(
            buf_pool,
            name,
            (GLfloat*)vertices.data(),
            vertices.size(),
            true,
            false,
            indices.data(),
            indices.size()
        )
    );
}

const primitive& ensure_patched_sphere_primitive(
    resource_pool& pool,
    unsigned subdivisions
){
    return ensure_patched_sphere_primitive(pool, pool, subdivisions);
}

const texture& ensure_circular_random_texture(
    texture_pool& pool,
    glm::uvec2 size
){
    std::string name =
        "circular_random_" + std::to_string(size.x)
        + "x" + std::to_string(size.y);

    if(pool.contains(name)) return *pool.get(name);
    return *pool.add(name, generate_circular_random(pool.get_context(), size));
}

const texture& ensure_circular_poisson_texture(
    texture_pool& pool,
    unsigned size
){
    std::string name = "circular_poisson_" + std::to_string(size);

    if(pool.contains(name)) return *pool.get(name);
    return *pool.add(name, generate_circular_poisson(pool.get_context(), size));
}

const texture& ensure_spherical_random_texture(
    texture_pool& pool,
    glm::uvec2 size
){
    std::string name =
        "spherical_random_" + std::to_string(size.x)
        + "x" + std::to_string(size.y);

    if(pool.contains(name)) return *pool.get(name);
    return *pool.add(name, generate_spherical_random(pool.get_context(), size));
}

const texture& ensure_spherical_poisson_texture(
    texture_pool& pool,
    unsigned size
){
    std::string name = "spherical_poisson_" + std::to_string(size);

    if(pool.contains(name)) return *pool.get(name);
    return *pool.add(name, generate_spherical_poisson(pool.get_context(), size));
}

} // namespace lt::common
