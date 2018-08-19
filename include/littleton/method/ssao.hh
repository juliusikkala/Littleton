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
#ifndef LT_METHOD_SSAO_HH
#define LT_METHOD_SSAO_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../primitive.hh"
#include "../sampler.hh"
#include "../doublebuffer.hh"
#include "../resource.hh"
#include "../scene.hh"
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

LT_OPTIONS(ssao)
{
    float radius = 0.2f;
    unsigned samples = 16;
    unsigned blur_radius = 1;
    float bias = 0.01f;
};

class LT_API ssao:
    public target_method,
    public scene_method<camera_scene, light_scene>,
    public options_method<ssao>,
    public glresource
{
public:
    ssao(
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
    gbuffer* buf;

    shader* ssao_shader;
    shader* vertical_blur_shader;
    shader* horizontal_blur_shader;
    multishader* ambient_shader;

    doublebuffer ssao_buffer;

    const texture& random_rotation;
    std::unique_ptr<texture> kernel;

    const primitive& quad;
    const sampler& fb_sampler;
    sampler noise_sampler;
};

} // namespace lt::method

#endif
