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
#ifndef LT_METHOD_SAO_HH
#define LT_METHOD_SAO_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../primitive.hh"
#include "../sampler.hh"
#include "../framebuffer.hh"
#include "../doublebuffer.hh"
#include "../resource.hh"
#include "shadow_method.hh"
#include <memory>

namespace lt
{

class gbuffer;
class resource_pool;
class shader;
class multishader;

}

namespace lt::method
{

// Scalable ambient obsurance (McGuire, HPG 2012)
class LT_API sao: public target_method, public glresource
{
public:
    sao(
        render_target& target,
        gbuffer& buf,
        resource_pool& pool,
        render_scene* scene,
        float radius = 0.5f,
        unsigned samples = 8,
        float bias = 0.01f,
        float intensity = 1.0f
    );

    void set_scene(render_scene* scene);
    render_scene* get_scene() const;

    void set_radius(float radius);
    float get_radius() const;

    void set_samples(unsigned samples);
    unsigned get_samples() const;

    void set_bias(float bias);
    float get_bias() const;

    void set_intensity(float intensity);
    float get_intensity() const;

    void execute() override;

    std::string get_name() const override;

private:
    gbuffer* buf;

    shader* ao_sample_pass_shader;
    shader* blur_shader;
    multishader* ambient_shader;
    render_scene* scene;

    float radius;
    unsigned samples;
    float bias;
    float intensity;
    float spiral_turns;

    doublebuffer ao;

    const primitive& quad;
    const sampler& fb_sampler;
    sampler mipmap_sampler;
};

} // namespace lt::method

#endif

