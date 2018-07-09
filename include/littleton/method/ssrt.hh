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
#include "../stencil_handler.hh"
#include <memory>

namespace lt
{

class gbuffer;
class resource_pool;
class multishader;
class shader;
class render_scene;

}

namespace lt::method
{

class LT_API ssrt: public target_method, public stencil_handler
{
public:
    ssrt(
        render_target& target,
        gbuffer& buf,
        resource_pool& pool,
        render_scene* scene,
        float roughness_cutoff = 0.2f
    );

    void set_scene(render_scene* scene);
    render_scene* get_scene() const;

    // Limiting max steps improves worst-case performance, but limits
    // reflected range near geometry boundaries. When thickness is finite,
    // this should be larger. 
    void set_max_steps(unsigned max_steps = 500);
    void set_roughness_cutoff(float cutoff = 0.5f);
    void set_brdf_cutoff(float cutoff = 0.0f);
    // Set to negative for infinite depth (faster)
    void set_thickness(float thickness = -1.0f);
    // Distance of the first sample from the point where the ray starts
    void set_ray_offset(float offset = 0.01f);

    void use_fallback_cubemap(bool use = true);

    void execute() override;

    std::string get_name() const override;

private:
    void refresh_shader();

    gbuffer* buf;
    resource_pool& pool;

    multishader* ssrt_shaders;
    shader* ssrt_shader;
    shader* ssrt_shader_env;
    shader* blit_shader;

    render_scene* scene;

    const primitive& quad;
    const sampler& fb_sampler;
    sampler mipmap_sampler;
    sampler cubemap_sampler;

    unsigned max_steps;
    float thickness;
    float roughness_cutoff;
    float brdf_cutoff;
    float ray_offset;
    bool fallback_cubemap;
};

} // namespace lt::method

#endif
