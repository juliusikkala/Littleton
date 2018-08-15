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

LT_OPTIONS(apply_sg)
{
    // If the roughness of the surface is less than this, the specular part
    // won't be applied to it (this is useful if you mix different reflection
    // methods such as cubemaps and SSRT)
    float min_specular_roughness = 0.2f;
};

class LT_API apply_sg:
    public target_method,
    public scene_method<camera_scene, sg_scene>,
    public options_method<apply_sg>,
    public glresource, public stencil_handler
{
public:
    apply_sg(
        render_target& target,
        gbuffer& buf,
        resource_pool& pool,
        Scene scene,
        const options& opt = {}
    );

    void execute() override;

    std::string get_name() const override;

private:
    gbuffer* buf;

    multishader* sg_shader;

    const primitive& cube;
    const sampler& fb_sampler;
    const sampler& linear_sampler;
};

} // namespace lt::method

#endif
