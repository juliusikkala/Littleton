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
#ifndef LT_METHOD_DRAW_SDF_HH
#define LT_METHOD_DRAW_SDF_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../primitive.hh"
#include "../shader.hh"
#include "../sampler.hh"
#include "../scene.hh"
#include "../sdf.hh"
#include "../transformable.hh"
#include "../stencil_handler.hh"
#include <string>
#include <vector>

namespace lt
{

class resource_pool;
class multishader;
class gbuffer;

}

namespace lt::method
{

LT_OPTIONS(draw_sdf)
{
    // Whether to apply ambient lighting or not. Disable if you are using
    // SSAO or other methods which add the ambient term on their own.
    bool apply_ambient = true;

    // 'insert' is inserted before any of the sdf_object functions
    std::string insert = "";
};

// Currently, this only works with a deferred pipeline. Sorry about that.
// (shadow maps man, they're not fun)
class LT_API draw_sdf:
    public target_method,
    public scene_method<
        camera_scene,
        sdf_scene
    >,
    public options_method<draw_sdf>,
    public stencil_handler
{
public:
    draw_sdf(
        gbuffer& target,
        resource_pool& pool,
        Scene scene,
        const options& opt = {}
    );

    void execute() override;

    std::string get_name() const override;

protected:
    void options_will_update(const options& next);

private:
    void update_sdf_shader();

    gbuffer* gbuf;
    resource_pool& pool;

    multishader* sdf_shaders;
    shader* sdf_shader;

    const primitive& quad;
    const sampler& linear_sampler;        
    const sampler& fb_sampler;
    sampler mipmap_sampler;
};

} // namespace lt::method

#endif

