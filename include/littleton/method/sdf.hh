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
#include "../primitive.hh"
#include "../shader.hh"
#include "../sampler.hh"
#include "../transformable.hh"
#include "../stencil_handler.hh"
#include <string>
#include <vector>

namespace lt
{

class resource_pool;
class render_scene;
class multishader;
class material;
class gbuffer;

/* TODO: Move SDFs to the scene (sdf_scene or something? Might need scene
 * refactoring)
 *
 * 'distance' must be the contents of a glsl function with the following
 * definition:
 *
 * float distance_func(in vec3 p)
 * {
 *     <your code> // Your code must return a float
 * }
 *
 * If texture_mapping is defined, the textures of the given material are
 * used. 'texture_mapping' must be the contents of a glsl function with the
 * following definition:
 *
 * void texture_mapping(
 *     in vec3 p,
 *     in vec3 pdx,
 *     in vec3 pdy,
 *     out vec2 uv,
 *     out vec2 uvdx,
 *     out vec2 uvdy
 * ) {
 *     <your code>
 * }
 */
class sdf_object: public transformable_node
{
public:
    sdf_object(
        const material* mat,
        const std::string& distance,
        const std::string& texture_mapping = ""
    );

    void set_material(const material* mat);
    const material* get_material() const;

    void set_distance(const std::string& distance);
    const std::string& get_distance() const;

    void set_texture_mapping(const std::string& texture_mapping);
    const std::string& get_texture_mapping() const;

private:
    const material* mat;
    std::string distance;
    std::string texture_mapping;
};

}

namespace lt::method
{

// Currently, this only works with a deferred pipeline. Sorry about that.
// (shadow maps man, they're not fun)
class LT_API sdf: public target_method, public stencil_handler
{
public:
    // 'insert' is inserted before any of the sdf_object functions
    sdf(
        gbuffer& target,
        resource_pool& pool,
        render_scene* scene,
        const std::vector<sdf_object>& sdfs = {},
        bool apply_ambient = true,
        const std::string& insert = ""
    );

    void set_scene(render_scene* scene);
    render_scene* get_scene() const;

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

    render_scene* scene;
    bool apply_ambient;
    const primitive& quad;
    const sampler& linear_sampler;        
    const sampler& fb_sampler;
    sampler mipmap_sampler;

    std::string insert;
    std::vector<sdf_object> sdfs;
};

} // namespace lt::method

#endif

