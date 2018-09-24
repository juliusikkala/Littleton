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
#include "method/sdf.hh"
#include "primitive.hh"
#include "gbuffer.hh"
#include "sampler.hh"
#include "multishader.hh"
#include "camera.hh"
#include "resource_pool.hh"
#include "common_resources.hh"

namespace lt::method
{

render_sdf::render_sdf(
    gbuffer& buf,
    resource_pool& pool,
    Scene scene,
    const options& opt
):  target_method(buf),
    scene_method(scene),
    options_method(opt),
    sdf_shader(pool.get_shader(shader::path{"cast_ray.vert", "sdf.frag"})),
    fb_sampler(common::ensure_framebuffer_sampler(pool)),
    quad(common::ensure_quad_primitive(pool))
{
}

void render_sdf::execute()
{
    target_method::execute();
    if(!sdf_shader || !has_all_scenes())
        return;

    auto [
        apply_ambient, apply_lighting, render_transparent, num_refractions,
        num_reflections, write_depth, max_steps, min_dist, max_dist, step_ratio,
        hit_ratio
    ] = opt;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    stencil_draw();

    light_scene* lights = get_scene<light_scene>();
    camera* cam = get_scene<camera_scene>()->get_camera();
    if(!cam) return;

    glm::mat4 p = cam->get_projection();
    glm::mat4 ip = glm::inverse(cam->get_projection());

    shader::definition_map def({
        {"LOCAL_VIEW_DIR", ""},
        {"VERTEX_NORMAL", ""},
        {"FUNCTIONS", ""}
    });

    gbuffer* gbuf = static_cast<gbuffer*>(&get_target());

    gbuf->set_draw(gbuffer::DRAW_ALL);
    gbuf->update_definitions(def);

    shader* s = sdf_shader->get(def);
    s->bind();

    s->set("ambient", lights->get_ambient());
    s->set("ivp", toMat4(cam->get_global_orientation()) * ip);
    s->set("ip", ip);
    s->set("proj", p);
    s->set("projection_info", cam->get_projection_info());
    s->set("clip_info", cam->get_clip_info());
    s->set("near", -cam->get_near());

    quad.draw();

    gbuf->set_draw(gbuffer::DRAW_LIGHTING);
}

}
