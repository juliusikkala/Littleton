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
#ifndef LT_METHOD_GENERATE_SG_HH
#define LT_METHOD_GENERATE_SG_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../gpu_buffer.hh"
#include "../framebuffer.hh"
#include "../scene.hh"
#include "../shader.hh"
#include "../spherical_gaussians.hh"
#include "../resource.hh"
#include "forward_pass.hh"
#include "skybox.hh"
#include <unordered_map>
#include <boost/functional/hash.hpp>

namespace lt
{

class resource_pool;

}

namespace lt::method
{

class LT_API generate_sg:
    public pipeline_method,
    public scene_method<
        object_scene,
        light_scene,
        shadow_scene,
        environment_scene,
        sg_scene
    >,
    public glresource
{
public:
    // Due to the nature of the method, the parameters 'resolution', 'samples'
    // and 'batch_size' don't use LT_OPTIONS. Consider them as immutable.
    generate_sg(
        resource_pool& pool,
        Scene scene,
        unsigned resolution = 16,
        unsigned samples = 8,
        unsigned batch_size = 32
    );

    void execute() override;

    texture* get_design_matrix(const sg_group& group);

private:
    struct least_squares_matrices
    {
        least_squares_matrices(
            context& ctx,
            unsigned batch_size,
            unsigned lobe_count,
            unsigned resolution,
            const std::vector<float>& x,
            const std::vector<float>& r
        );
        texture x; // Design matrix (cube map array)
        gpu_buffer xy; // Result of XTy for the whole batch
        gpu_buffer r; // Cholesky decomposition of X^T*X
    };

    least_squares_matrices& get_matrices(
        const sg_group& group
    );

    multishader* lobe_product;
    multishader* solve;
    multishader* copy;

    unsigned resolution;
    unsigned batch_size;
    camera_scene probe_cameras;
    framebuffer cubemap_probes;

    skybox sb;
    forward_pass fp;
    pipeline probe_pipeline;

    std::unordered_map<
        std::vector<sg_lobe>,
        least_squares_matrices,
        boost::hash<std::vector<sg_lobe>>
    > matrix_cache;
};

} // namespace lt::method
#endif

