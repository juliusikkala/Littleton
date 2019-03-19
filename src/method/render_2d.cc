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

    if(!draw_shader || !has_all_scenes()) return;

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_CULL_FACE);

    /*
    camera* cam = get_scene<camera_scene>()->get_camera();
    if(!cam) return;

    glm::mat4 vp =
        cam->get_projection() * glm::inverse(cam->get_global_transform());
    */
}

}
