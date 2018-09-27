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
#ifndef LT_METHOD_SDF_HH
#define LT_METHOD_SDF_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../shader.hh"
#include "../scene.hh"
#include "../sdf.hh"
#include "../stencil_handler.hh"

namespace lt
{

class texture;
class resource_pool;
class multishader;
class gbuffer;
class primitive;
class sampler;

}

namespace lt::method
{

LT_OPTIONS(render_sdf)
{
    bool apply_ambient = true;

    // Essentially enables forward shading. If false, but render_transparent is
    // enabled, only a single layer of transparency is rendered.
    bool apply_lighting = false;

    // Whether to render transparent objects only, with transmittance. This is
    // an on/off switch due to how the lighting pass is separate.
    bool render_transparent = false;
 
    // Disables writing to the depth buffer, you may want this for fog or
    // something. apply_lighting must be true for this to work.
    bool write_depth = true;

    // Works only when using the dual gbuffer constructor.
    unsigned num_refractions = 0;

    // Currently not implemented. Creates cross reflections between objects.
    // Incompatible with SSRT, but essentially replaces it.
    unsigned num_reflections = 0;

    // Maximum ray march steps to be taken. Higher values give better quality
    // with worse worst-case performance.
    unsigned max_steps = 64;

    // Minimum distance of the ray from its origin.
    float min_dist = 0.1f;

    // Maximum distance of the ray from its origin before it's declared a miss.
    // Lower values cause better performance. Essentially equivalent to render
    // distance.
    float max_dist = 20.0f;

    // Scale the step size down if you have geometry artifacts (range: (0, 1])
    // Lower values cause worse performance.
    float step_ratio = 0.999f;

    // If distance_to_surface < travelled_distance * hit_ratio, the ray has hit
    // the surface. Lower this value to get more accurate results with worse
    // performance.
    float hit_ratio = 0.001f;
};

class LT_API render_sdf:
    public target_method,
    public scene_method<camera_scene, sdf_scene, light_scene>,
    public options_method<render_sdf>,
    public animated_method,
    public stencil_handler
{
public:
    render_sdf(
        gbuffer& buf,
        resource_pool& pool,
        Scene scene,
        const options& opt = {}
    );

    void execute() override;

private:
    multishader* sdf_shader;
    const sampler& fb_sampler;
    const primitive& quad;
};

} // namespace lt::method

#endif

