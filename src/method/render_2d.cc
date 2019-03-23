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
#include "gbuffer.hh"
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

namespace
{
using namespace lt;

inline void apply_default_sampler(material::sampler_tex& st, const sampler* s)
{
    if(st.second && !st.first) st.first = s;
}

}

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
    quad(common::ensure_quad_nt_primitive(pool)),
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
    light_scene* ls = get_scene<light_scene>();

    if(!draw_shader || !cs) return;

    auto [
        read_depth_buffer, fullbright, write_buffer_data, default_emissive,
        perspective_orientation, apply_ambient
    ] = opt;

    // TODO: Set perspective_orientation to false if ortho camera.

    camera* cam = cs->get_camera();
    if(!cam) return;

    mat4 inverse_view_mat = cam->get_global_transform();
    mat4 view_mat = glm::inverse(inverse_view_mat);
    mat4 projection = cam->get_projection();
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

            cmd.mat = s->get_material();

            // If missing texture, skip.
            const texture *tex = cmd.mat.color_texture.second;
            if(!tex) continue;

            interpolation mag, min;
            s->get_interpolation(mag, min);
            const sampler* default_sampler = fetch_sampler(mag, min);
            apply_default_sampler(cmd.mat.color_texture, default_sampler);
            apply_default_sampler(
                cmd.mat.metallic_roughness_texture,
                default_sampler
            );
            apply_default_sampler(cmd.mat.normal_texture, default_sampler);
            apply_default_sampler(cmd.mat.emission_texture, default_sampler);

            quat ori = get_matrix_orientation(model);
            vec3 v = perspective_orientation ? cam_location - pos : view;
            vec3 mv = vec3(inverse(ori) * vec4(v, 0));

            bool cap = false;
            sprite_layout::tile tile = s->get_tile(mv, cap);
            cmd.uv_bounds = tile.rect;
            vec2 scaling = (
                vec2(tile.rect.z, tile.rect.w) - vec2(tile.rect.x, tile.rect.y)
            ) * vec2(tex->get_size()) * vec2(get_matrix_scaling(model));

            vec3 u = vec3(ori * (cap ? vec4(0,0,-1,0) : vec4(0,1,0,0)));
            vec2 cs = vec2(dot(u, up), dot(u, right));
            // Guard singularity when viewed from directly above.
            cs = dot(cs, cs) < 0.0001 ? vec2(1, 0) : normalize(cs);
            
            // TODO: Optimize this pile of matrix multiplications
            mat4 origin_translation = glm::translate(
                vec3(vec2(1.0f)-2.0f*tile.origin, 0)
            );
            mat4 scaling_matrix = glm::scale(vec3(scaling*0.5f, 1));
            mat4 rotation{
                cs.x, -cs.y, 0, 0,
                cs.y, cs.x, 0, 0,
                0,0,1,0,
                0,0,0,1
            };
            mat4 final_translation = glm::translate(
                vec3(view_mat * vec4(pos, 1))
            );

            cmd.mv = final_translation * rotation *
                scaling_matrix * origin_translation;
            cmd.n_m = inverseTranspose(cmd.mv);
            cmd.mvp = projection * cmd.mv;

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
    glDisable(GL_CULL_FACE);

    stencil_draw();

    if(!read_depth_buffer) glDepthFunc(GL_ALWAYS);
    if(!write_buffer_data) glDepthMask(GL_FALSE);

    shader::definition_map common({
        {"OUTPUT_LIGHTING", ""},
        {"UV_TRANSFORM", ""},
    });

    if(gbuf && write_buffer_data)
    {
        glDisable(GL_BLEND);
        common["MIN_ALPHA"] = "0.5f";
        common["OUTPUT_GEOMETRY"];
        common["APPLY_EMISSION"];
        gbuf->set_draw(gbuffer::DRAW_ALL);
        gbuf->update_definitions(common);
    }
    else
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    quad.update_definitions(common);


    if(!fullbright && apply_ambient && ls) common["APPLY_AMBIENT"];
    if(fullbright) common["FULLBRIGHT"];

    vec3 ambient = ls ? ls->get_ambient() : vec3(0);

    for(command& cmd: command_buffer)
    {
        shader::definition_map definitions(common);
        cmd.mat.update_definitions(definitions);

        shader* s = draw_shader->get(definitions);
        s->bind();

        s->set("mvp", cmd.mvp);
        s->set("m", cmd.mv);
        s->set("n_m", cmd.n_m);
        s->set("ambient", ambient);
        s->set("uv_offset", vec2(cmd.uv_bounds));
        s->set(
            "uv_scale",
            vec2(cmd.uv_bounds.z, cmd.uv_bounds.w)-vec2(cmd.uv_bounds)
        );

        unsigned texture_index = 0;
        cmd.mat.apply(s, texture_index);
        quad.draw();
    }

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);

    if(gbuf) gbuf->set_draw(gbuffer::DRAW_LIGHTING);
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
