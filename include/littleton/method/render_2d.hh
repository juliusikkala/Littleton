/*
    Copyright 2018-2019 Julius Ikkala

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
#ifndef LT_METHOD_RENDER_2D_HH
#define LT_METHOD_RENDER_2D_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../stencil_handler.hh"
#include "../scene.hh"
#include "../sampler.hh"
#include <unordered_map>
#include <boost/functional/hash.hpp>

namespace lt
{

class resource_pool;
class multishader;
class gbuffer;
class texture;
class sampler;
class material;
class primitive;

}

namespace lt::method
{

LT_OPTIONS(render_2d)
{
    // Whether to honor depth buffer data or not.
    // If you want the 2D world to interact with a 3D environment, enable this.
    bool read_depth_buffer = false;

    // If disabled, sprites will appear black. You probably want to enable
    // write_buffer_data if you disable this, so that you can use deferred
    // lighting with 2D items.
    bool fullbright = true;

    // Whether to write feature and depth buffers to the render target or not.
    // Keep this disabled for GUI and true 2D things, it's only potentially
    // useful for impostor sprites. If you don't want deferred lighting on the
    // sprites, don't enable this.
    bool write_buffer_data = false;

    // If write_buffer_data is enabled, this chooses whether materialless 2d
    // objects appear emissive or diffuse only.
    bool default_emissive = true;

    // If enabled, uses sprite location and camera-specific view direction for
    // orientation lookup. Otherwise, camera direction is used. If the camera
    // is using orthographic projection, this does nothing, since both the
    // camera direction and sprite-camera vector are the same.
    bool perspective_orientation = true;
};

class LT_API render_2d:
    public target_method,
    public scene_method<camera_scene, sprite_scene>,
    public options_method<render_2d>,
    public stencil_handler
{
public:
    render_2d(
        render_target& target,
        resource_pool& pool,
        Scene scene,
        const options& opt = {}
    );

    render_2d(
        gbuffer& target,
        resource_pool& pool,
        Scene scene,
        const options& opt = {}
    );

    ~render_2d();

    void execute() override;

private:
    const sampler* fetch_sampler(interpolation mag, interpolation min);

    resource_pool& pool;
    const primitive& quad;

    multishader* draw_shader;
    gbuffer* gbuf;

    struct command
    {
        float depth;
        const material* mat;
        const texture* tex;
        const sampler* default_sampler;
        vec4 uv_bounds;
        mat4 mvp;
    };
    // Stored here to avoid constant memory reallocation.
    std::vector<command> command_buffer;

    std::unordered_map<
        std::pair<int, int>,
        const sampler*,
        boost::hash<std::pair<int, int>>
    > sampler_cache;
};

} // namespace lt::method

#endif

