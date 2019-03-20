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
#include "render_2d.hh"
#include "multishader.hh"
#include "camera.hh"
#include "helpers.hh"
#include "resource_pool.hh"
#include "common_resources.hh"
#include "scene.hh"
#include "sprite.hh"
#include "texture.hh"
#include "sampler.hh"
#include <stdexcept>
#include <algorithm>

namespace lt::method
{

render_2d::render_2d(
    render_target& target,
    resource_pool& pool,
    Scene scene,
    const options& opt
):  target_method(target),
    scene_method(scene),
    options_method(opt),
    pool(pool),
    quad(common::ensure_quad_primitive(pool)),
    draw_shader(pool.get_shader(
        shader::path{"generic.vert", "forward.frag"}
    )),
    gbuf(nullptr)
{}

render_2d::render_2d(
    gbuffer& target,
    resource_pool& pool,
    Scene scene,
    const options& opt
): render_2d((render_target&)target, pool, scene, opt)
{
    gbuf = &target;
}

render_2d::~render_2d() {}

void render_2d::execute()
{
    target_method::execute();

    camera_scene* cs = get_scene<camera_scene>();
    sprite_scene* ss = get_scene<sprite_scene>();

    if(!draw_shader || !cs) return;

    auto [
        read_depth_buffer, fullbright, write_buffer_data, default_emissive,
        perspective_orientation
    ] = opt;

    // TODO: Set perspective_orientation to false if ortho camera.

    camera* cam = cs->get_camera();
    if(!cam) return;

    mat4 inverse_view_mat = cam->get_global_transform();
    quat cam_orientation = get_matrix_orientation(inverse_view_mat);
    vec3 cam_location = get_matrix_translation(inverse_view_mat);
    vec3 view = vec3(cam_orientation * vec4(0,0,-1,0));
    vec3 up = vec3(cam_orientation * vec4(0,1,0,0));
    vec3 right = vec3(cam_orientation * vec4(1,0,0,0));

    // Compute transforms & final depth of all 2d objects.
    command_buffer.clear();

    if(ss)
    {
        for(sprite* s: ss->get_sprites())
        {
            mat4 model = s->get_global_transform();
            vec3 pos = get_matrix_translation(model);

            command cmd;
            cmd.depth = dot(cam_location - pos, view);
            // Skip if behind camera.
            if(cmd.depth > 0) continue;

            quat ori = get_matrix_orientation(model);
            vec3 v = perspective_orientation ? cam_location - pos : view;
            vec3 mv = vec3(inverse(ori) * vec4(v, 0));

            bool cap = false;
            sprite_layout::tile tile = s->get_tile(mv, cap);
            cmd.uv_bounds = tile.rect;

            vec3 u = vec3(ori * (cap ? vec4(0,0,-1,0) : vec4(0,1,0,0)));
            vec2 cs = vec2(dot(u, up), dot(u, right));
            // Guard singularity when viewed from directly above.
            cs = dot(cs, cs) < 0.0001 ? vec2(1, 0) : normalize(cs);

            cmd.mat = &s->get_material();
            cmd.tex = nullptr;

            interpolation mag, min;
            s->get_interpolation(mag, min);
            cmd.default_sampler = fetch_sampler(mag, min);

            // TODO: Determine final transform based on 'cs' (cosine and sine
            // for rotation), pos, scale and tile origin.

            command_buffer.push_back(cmd);
        }
    }

    std::sort(
        command_buffer.begin(),
        command_buffer.end(),
        [](const command& a, const command& b){ return a.depth < b.depth; }
    );

    // Render sprites
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_CULL_FACE);

    // TODO: Render command buffer.
}

const sampler* render_2d::fetch_sampler(interpolation mag, interpolation min)
{
    std::pair<int, int> key = { static_cast<int>(mag), static_cast<int>(min) };
    auto it = sampler_cache.find(key);
    if(it == sampler_cache.end())
    {
        const sampler* s = &common::ensure_generic_sampler(pool, mag, min);
        sampler_cache.emplace(key, s);
        return s;
    }
    return it->second;
}

} // namespace lt::method
