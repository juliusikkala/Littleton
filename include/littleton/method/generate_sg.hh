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
#include "../primitive.hh"
#include "../framebuffer.hh"
#include "../scene.hh"
#include "../spherical_gaussians.hh"
#include "forward_pass.hh"
#include <boost/functional/hash.hpp>

namespace lt
{

class resource_pool;

}

namespace lt::method
{

class LT_API generate_sg: public pipeline_method
{
public:
    generate_sg(
        resource_pool& pool,
        render_scene* scene,
        size_t resolution=16,
        // Batches can't be very large, because even high-end GPUs at the time
        // of writing (1080 ti) are hitting geometry shader vertex limits at
        // batch_size == 3 :/
        size_t batch_size=2
    );

    void set_scene(render_scene* scene);

    void execute() override;

    std::string get_name() const override;

private:
    struct least_squares_matrices
    {
        std::vector<float> x; // Design matrix
        std::vector<float> r; // Cholesky decomposition of X^T*X
    };

    least_squares_matrices& get_matrices(
        const sg_group& group
    );

    render_scene* scene;
    size_t resolution;
    size_t batch_size;
    render_scene probe_scene;
    framebuffer cubemap_probes;
    forward_pass fp;

    std::unordered_map<
        std::vector<sg_lobe>,
        least_squares_matrices,
        boost::hash<std::vector<sg_lobe>>
    > matrix_cache;
};

} // namespace lt::method
#endif

