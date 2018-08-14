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
#include "../scene.hh"
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

LT_OPTIONS(sao)
{
    float radius = 0.5f;
    unsigned samples = 8;
    float bias = 0.01f;
    float intensity = 1.0f;
};

// Scalable ambient obsurance (McGuire, HPG 2012)
class LT_API sao:
    public target_method,
    public scene_method<camera_scene, light_scene>,
    public options_method<sao>,
    public glresource
{
public:
    sao(
        render_target& target,
        gbuffer& buf,
        resource_pool& pool,
        Scene scene,
        const options& opt = {}
    );

    void execute() override;

    std::string get_name() const override;

protected:
    void options_will_update(const options& next, bool initial = false);

private:
    gbuffer* buf;

    shader* ao_sample_pass_shader;
    shader* blur_shader;
    multishader* ambient_shader;

    float spiral_turns;

    doublebuffer ao;

    const primitive& quad;
    const sampler& fb_sampler;
    sampler mipmap_sampler;
};

} // namespace lt::method

#endif

