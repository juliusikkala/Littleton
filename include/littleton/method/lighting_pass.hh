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
#ifndef LT_METHOD_LIGHTING_PASS_HH
#define LT_METHOD_LIGHTING_PASS_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../primitive.hh"
#include "../sampler.hh"
#include "../scene.hh"
#include "../stencil_handler.hh"
#include "shadow_method.hh"

namespace lt
{

class gbuffer;
class resource_pool;
class multishader;

}

namespace lt::method
{


LT_OPTIONS(lighting_pass)
{
    // Set to negative to not use light volumes
    float cutoff = 5/256.0f;

    enum depth_test
    {
        TEST_NONE = 0,
        TEST_NEAR,
        TEST_FAR
    };

    // Type of depth culling for the light volumes
    depth_test test = TEST_NEAR;

    // If true, highlights the light volumes by drawing them slightly brighter
    bool visualize_light_volumes = false;
};

class shadow_method;
class LT_API lighting_pass:
    public target_method,
    public scene_method<camera_scene, light_scene, shadow_scene>,
    public options_method<lighting_pass>,
    public stencil_handler
{
public:
    lighting_pass(
        render_target& target,
        gbuffer& buf,
        resource_pool& pool,
        Scene scene,
        const options& opt = {}
    );

    void execute() override;

private:
    gbuffer* buf;

    multishader* ambient_shader;
    multishader* lighting_shader;

    const primitive& quad;
    const sampler& fb_sampler;
};

} // namespace lt::method

#endif
