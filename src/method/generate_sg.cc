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

namespace 
{
    lt::vec3 orientate_vector(
        lt::vec3 dir,//assuming z-positive
        int cubemap_face
    ){
        switch(cubemap_face)
        {
        case 0:
            return lt::vec3(dir.z, dir.y, -dir.x);
        case 1:
            return lt::vec3(-dir.z, dir.y, dir.x);
        case 2:
            return lt::vec3(dir.x, dir.z, -dir.y);
        case 3:
            return lt::vec3(dir.x, -dir.z, dir.y);
        default:
        case 4:
            return dir;
        case 5:
            return lt::vec3(-dir.x, dir.y, -dir.z);
        }
    }

    lt::vec3 index_position(unsigned index, lt::uvec3 space)
    {
        unsigned layer = space.x * space.y;

        unsigned z = index / (layer);
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
    resolution(resolution),
    batch_size(batch_size),
    cubemap_probes(
        pool.get_context(),
        glm::uvec3(resolution, resolution, batch_size),
        {{GL_DEPTH_ATTACHMENT, {GL_DEPTH_COMPONENT16}},
         {GL_COLOR_ATTACHMENT0, {GL_RGB16F, true}}},
        0,
        GL_TEXTURE_CUBE_MAP_ARRAY
    ),
    fp(
        cubemap_probes,
        pool,
        &probe_scene,
        false,
        false
    )
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
    camera* view_camera = scene->get_camera();
    float near = 0.1f;
    float far = 100.0f;
    if(view_camera)
    {
        near = view_camera->get_near();
        far = view_camera->get_far();
    }

    std::vector<camera> batch_cameras(batch_size);
    for(camera& c: batch_cameras) c.cube_perspective(near, far);

    for(const sg_group* sg: scene->get_sg_groups())
    {
        generate_sg::least_squares_matrices& m = get_matrices(*sg);
        mat4 transform = sg->get_global_transform();

        uvec3 res = sg->get_resolution();
        size_t probes = res.x*res.y*res.z;
        size_t i = 0;

        // Go through the probes in batches
        while(i < probes)
        {
            // Clear framebuffer
            cubemap_probes.bind();
            glClear(
                GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT
            );
            
            size_t batch_probes = min(probes-i, batch_size);

            std::vector<camera*> c;
            for(unsigned j = 0; j < batch_probes; ++j, ++i)
            {
                vec3 cam_pos(transform * vec4(index_position(i, res), 1));
                batch_cameras[j].set_position(cam_pos);
                c.push_back(&batch_cameras[j]);
            }

            probe_scene.set_cameras(c);
            fp.execute();

            // TODO: compute spherical gaussians from cubemap probes
        }
    }
}

std::string generate_sg::get_name() const
{
    return "generate_sg";
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
    for(size_t p = 0; p < total_pixels; ++p)
    {
        size_t face_p = p % face_pixels;
        size_t face_index = p / face_pixels;
        size_t face_x = face_p % resolution;
        size_t face_y = face_p / resolution;

        // dir is the direction of the vector for this sample
        vec3 dir = vec3(face_x, face_y, 0);
        dir = (dir - (resolution - 1) * 0.5f)/(float)resolution;
        dir = orientate_vector(normalize(vec3(dir.x, -dir.y, 1)), face_index);

        for(size_t l = 0; l < lobes.size(); ++l)
        {
            // Apply the function (spherical gaussian) to the direction
            float value = exp(
                lobes[l].sharpness * (dot(lobes[l].axis, dir) - 1.0f)
            );
            x[p * lobes.size() + l] = value;
        }
    }

    // Cholesky decomposition of X^T*X
    matrix_transpose_product(
        x.data(),
        total_pixels,
        lobes.size(),
        r.data()
    );
    cholesky_decomposition(r.data(), lobes.size());
    auto p = matrix_cache.try_emplace(lobes, get_context(), x, r);
    return p.first->second;
}

generate_sg::least_squares_matrices::least_squares_matrices(
    context& ctx,
    const std::vector<float>& x,
    const std::vector<float>& r
):  x(ctx, GL_SHADER_STORAGE_BUFFER, x.size()*sizeof(float), x.data()),
    r(ctx, GL_SHADER_STORAGE_BUFFER, r.size()*sizeof(float), r.data())
{
}

}
