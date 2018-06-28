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

class LT_API generate_sg: public pipeline_method, public glresource
{
public:
    generate_sg(
        resource_pool& pool,
        render_scene* scene,
        size_t resolution=16,
        size_t batch_size=32
    );

    void set_scene(render_scene* scene);

    void execute() override;

    std::string get_name() const override;

    texture* get_design_matrix(const sg_group& group);

private:
    struct least_squares_matrices
    {
        least_squares_matrices(
            context& ctx,
            size_t batch_size,
            size_t lobe_count,
            size_t resolution,
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

    render_scene* scene;
    multishader* lobe_product;
    size_t resolution;
    size_t batch_size;
    render_scene probe_scene;
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

