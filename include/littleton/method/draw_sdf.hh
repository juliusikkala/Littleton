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

// Currently, this only works with a deferred pipeline. Sorry about that.
// (shadow maps man, they're not fun)
class LT_API draw_sdf:
    public target_method,
    public scene_method<
        camera_scene,
        sdf_scene
    >,
    public stencil_handler
{
public:
    // 'insert' is inserted before any of the sdf_object functions
    draw_sdf(
        gbuffer& target,
        resource_pool& pool,
        Scene scene,
        bool apply_ambient = true,
        const std::string& insert = ""
    );

    // Note that calling this functions resets the sdf_shader and is quite slow.
    // If you need to mutate your SDFs, please use get_shader() and set some
    // uniforms there. You can declare additional uniforms with the 'insert'
    // parameter of the constructor.
    void set_sdfs(const std::vector<sdf_object>& sdfs);
    const std::vector<sdf_object>& get_sdfs() const;

    // Whether to apply ambient lighting or not. Disable if you are using
    // SSAO or other methods which add the ambient term on their own.
    void set_apply_ambient(bool apply_ambient);
    bool get_apply_ambient() const;

    shader* get_shader();

    void execute() override;

    std::string get_name() const override;

private:
    void update_sdf_shader();

    gbuffer* gbuf;
    resource_pool& pool;

    multishader* sdf_shaders;
    shader* sdf_shader;

    bool apply_ambient;
    const primitive& quad;
    const sampler& linear_sampler;        
    const sampler& fb_sampler;
    sampler mipmap_sampler;

    std::string insert;
};

} // namespace lt::method

#endif

