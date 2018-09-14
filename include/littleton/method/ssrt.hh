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
#ifndef LT_METHOD_SSRT_HH
#define LT_METHOD_SSRT_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../primitive.hh"
#include "../sampler.hh"
#include "../doublebuffer.hh"
#include "../resource.hh"
#include "../scene.hh"
#include "../stencil_handler.hh"
#include <memory>

namespace lt
{

class gbuffer;
class resource_pool;
class multishader;
class shader;

}

namespace lt::method
{

LT_OPTIONS(ssrt)
{
    float roughness_cutoff = 0.2f;
    float brdf_cutoff = 0.0f;
    // Limiting max steps improves worst-case performance, but limits
    // reflected range near geometry boundaries. When thickness is finite,
    // this should be larger. 
    unsigned max_steps = 500;
    // Set to negative for infinite depth (faster)
    float thickness = -1.0f;
    // Distance of the first sample from the point where the ray starts
    float ray_offset = 0.1f;
    bool use_fallback_cubemap = true;
};

class LT_API ssrt:
    public target_method,
    public scene_method<camera_scene, environment_scene>,
    public options_method<ssrt>,
    public stencil_handler
{
public:
    ssrt(
        render_target& target,
        gbuffer& buf,
        resource_pool& pool,
        Scene scene,
        const options& opt = {}
    );

    void execute() override;

protected:
    void options_will_update(const options& next, bool initial = false);

private:
    void refresh_shader();

    gbuffer* buf;
    resource_pool& pool;

    multishader* ssrt_shaders;
    shader* ssrt_shader;
    shader* ssrt_shader_env;
    shader* blit_shader;

    const primitive& quad;
    const sampler& fb_sampler;
    sampler mipmap_sampler;
    sampler cubemap_sampler;
};

} // namespace lt::method

#endif
