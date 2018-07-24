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
#ifndef LT_METHOD_APPLY_SG_HH
#define LT_METHOD_APPLY_SG_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../primitive.hh"
#include "../sampler.hh"
#include "../framebuffer.hh"
#include "../doublebuffer.hh"
#include "../resource.hh"
#include "../stencil_handler.hh"
#include "../scene.hh"
#include "shadow_method.hh"
#include <memory>

namespace lt
{

class gbuffer;
class resource_pool;
class multishader;

}

namespace lt::method
{

class LT_API apply_sg:
    public target_method, public scene_method<camera_scene, environment_scene>,
    public glresource, public stencil_handler
{
public:
    apply_sg(
        render_target& target,
        gbuffer& buf,
        resource_pool& pool,
        Scene scene,
        float min_specular_roughness = 0.2f
    );

    void execute() override;

    std::string get_name() const override;

private:
    gbuffer* buf;

    multishader* sg_shader;

    float min_specular_roughness;
    const primitive& cube;
    const sampler& fb_sampler;
    const sampler& linear_sampler;
};

} // namespace lt::method

#endif
