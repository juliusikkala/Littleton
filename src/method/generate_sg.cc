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
#include "generate_sg.hh"
#include "resource_pool.hh"
#include "camera.hh"
#include "multishader.hh"

namespace 
{
    lt::vec3 index_position(unsigned index, lt::uvec3 space)
    {
        unsigned layer = space.x * space.y;

        unsigned z = index / layer;
        unsigned layer_index = index % layer;
        unsigned y = layer_index / space.x;
        unsigned x = layer_index % space.x;

        lt::vec3 pos(x, y, z);
        pos -= lt::vec3(space-lt::uvec3(1))*0.5f;
        pos /= lt::vec3(space);
        return pos;
    }
}

namespace lt::method
{

generate_sg::generate_sg(
    resource_pool& pool,
    render_scene* scene,
    size_t resolution,
    size_t batch_size
):  glresource(pool.get_context()),
    scene(scene),
    lobe_product(pool.get_shader(shader::path{"sg/lobe.comp"})),
    solve(pool.get_shader(shader::path{"sg/solve.comp"})),
    copy(pool.get_shader(shader::path{"sg/copy.comp"})),
    resolution(resolution),
    batch_size(batch_size),
    cubemap_probes(
        pool.get_context(),
        glm::uvec3(resolution, resolution, batch_size),
        {{GL_DEPTH_ATTACHMENT, {GL_DEPTH24_STENCIL8}},
         {GL_COLOR_ATTACHMENT0, {GL_RGBA16F, true}}},
        0,
        GL_TEXTURE_CUBE_MAP_ARRAY
    ),
    sb(cubemap_probes, pool, &probe_scene),
    fp(cubemap_probes, pool, &probe_scene, false, false),
    probe_pipeline({&sb, &fp})
{
}

void generate_sg::set_scene(render_scene* scene)
{
    this->scene = scene;
}

void generate_sg::execute()
{
    if(!scene) return;

    // Generate probe scene
    probe_scene = *scene;

    for(const sg_group* sg: scene->get_sg_groups())
    {
        std::vector<camera> batch_cameras(batch_size);
        for(camera& c: batch_cameras)
            c.cube_perspective(sg->get_near(), sg->get_far());

        generate_sg::least_squares_matrices& m = get_matrices(*sg);
        mat4 transform = sg->get_global_transform();

        uvec3 res = sg->get_resolution();

        size_t lobe_count = sg->get_lobes().size();
        shader::definition_map definitions({
            {"LOBE_COUNT", std::to_string(lobe_count)},
            {"IMAGE_RESOLUTION", std::to_string(resolution)},
            {"MAX_BATCH_SIZE", std::to_string(batch_size)}
        });
        shader* p = lobe_product->get(definitions);
        shader* s = solve->get(definitions);
        shader* c = copy->get(definitions);

        p->set_image_texture("input_weights", m.x, 0);
        p->set_image_texture(
            "input_maps",
            *cubemap_probes.get_texture_target(GL_COLOR_ATTACHMENT0), 1
        );
        p->set_storage_block("output_lobes", m.xy, 2);

        s->set_storage_block("inout_lobes", m.xy , 0);
        s->set_storage_block("r_matrix", m.r , 1);

        c->set_storage_block("input_lobes", m.xy , 0);

        size_t probes = res.x*res.y*res.z;
        size_t i = 0;

        // Go through the probes in batches
        cubemap_probes.bind();
        while(i < probes)
        {
            // Clear framebuffer
            glClearColor(0,0,0,0);
            glClearDepth(1);
            glClear(
                GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT
            );
            
            size_t batch_probes = min(probes-i, batch_size);

            std::vector<camera*> cameras;
            for(unsigned j = 0; j < batch_probes; ++j, ++i)
            {
                vec3 cam_pos(transform * vec4(index_position(i, res), 1));
                batch_cameras[j].set_position(cam_pos);
                batch_cameras[j].set_orientation(sg->get_global_orientation());
                cameras.push_back(&batch_cameras[j]);
            }

            probe_scene.set_cameras(cameras);
            probe_pipeline.execute();
            p->compute_dispatch(uvec3(batch_probes,1,1));

            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

            s->compute_dispatch(uvec3(batch_probes,1,1));

            glMemoryBarrier(
                GL_SHADER_STORAGE_BARRIER_BIT |
                GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
            );

            /*std::vector<vec4> sg_probes = m.xy.read<vec4>();
            for(unsigned j = 0; j < batch_probes; ++j)
            {
                printf("Probe %u\n", i-batch_probes+j);
                for(unsigned k = 0; k < lobe_count; ++k)
                {
                    printf("\tLobe %u: ", k);
                    vec4 value = sg_probes[j*lobe_count+k];
                    printf("rgb(%f, %f, %f)\n", value.x, value.y, value.z);
                }
            }*/

            c->set<int>("start_index", i - batch_probes);
            for(unsigned j = 0; j < lobe_count; ++j)
            {
                c->set<int>("lobe_index", j);
                c->set_image_texture(
                    "lobe_output", sg->get_amplitudes(j), 3, GL_WRITE_ONLY
                );
                c->compute_dispatch(uvec3(batch_probes,1,1));
            }
        }

        glMemoryBarrier(
            GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT |
            GL_TEXTURE_UPDATE_BARRIER_BIT
        );
    }

    glMemoryBarrier(
        GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT |
        GL_TEXTURE_UPDATE_BARRIER_BIT
    );
}

std::string generate_sg::get_name() const
{
    return "generate_sg";
}

texture* generate_sg::get_design_matrix(const sg_group& group)
{
    //return &get_matrices(group).x;
    return cubemap_probes.get_texture_target(GL_COLOR_ATTACHMENT0);
}

generate_sg::least_squares_matrices& generate_sg::get_matrices(
    const sg_group& group
){
    const std::vector<sg_lobe>& lobes = group.get_lobes();
    auto it = matrix_cache.find(lobes);
    if(it != matrix_cache.end())
        return it->second;

    // Not in cache, calculate the matrices. This can be slow, depending on
    // resolution and number of lobes.
    size_t face_pixels = resolution*resolution;
    size_t total_pixels = face_pixels*6;
    std::vector<float> x(total_pixels*lobes.size());
    std::vector<float> r(lobes.size()*lobes.size());

    // Generate design matrix
    for(size_t l = 0; l < lobes.size(); ++l)
    {
        for(size_t p = 0; p < total_pixels; ++p)
        {
            size_t face_p = p % face_pixels;
            size_t face_index = p / face_pixels;
            size_t face_x = face_p % resolution;
            size_t face_y = face_p / resolution;

            // dir is the direction of the vector for this sample
            vec3 dir = vec3(face_x, face_y, 0);
            dir = (dir + 0.5f) / (resolution*0.5f) - 1.0f;
            dir = swizzle_for_cube_face(
                normalize(vec3(dir.x, -dir.y, 1)),
                face_index
            );
            // Apply the function (spherical gaussian) to the direction
            float value = exp(
                lobes[l].sharpness * (dot(lobes[l].axis, dir) - 1.0f)
            );
            x[l * total_pixels + p] = value;
        }
    }

    // Compute X*X^T
    for(unsigned i = 0; i < lobes.size(); ++i)
    {
        for(unsigned j = 0; j <= i; ++j)
        {
            float& sum = r[i*lobes.size() + j] = 0;
            for(unsigned k = 0; k < total_pixels; ++k)
                sum += x[i*total_pixels + k] * x[j*total_pixels + k];
            r[j*lobes.size() + i] = sum;
        }
    }

    // Cholesky decomposition of X*X^T
    cholesky_decomposition(r.data(), lobes.size());
    auto p = matrix_cache.try_emplace(
        lobes,
        get_context(),
        batch_size,
        lobes.size(),
        resolution,
        x, r
    );
    return p.first->second;
}

generate_sg::least_squares_matrices::least_squares_matrices(
    context& ctx,
    size_t batch_size,
    size_t lobe_count,
    size_t resolution,
    const std::vector<float>& x,
    const std::vector<float>& r
):  x(
        ctx,
        uvec3(resolution, resolution, lobe_count),
        GL_R16F,
        GL_FLOAT,
        0,
        GL_TEXTURE_CUBE_MAP_ARRAY,
        x.data()
    ),
    xy(ctx, GL_SHADER_STORAGE_BUFFER, batch_size*lobe_count*sizeof(vec4)),
    r(ctx, GL_SHADER_STORAGE_BUFFER, r.size()*sizeof(float), r.data())
{
}

}
