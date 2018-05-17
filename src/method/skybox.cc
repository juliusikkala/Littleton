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
#include "skybox.hh"
#include "common_resources.hh"
#include "scene.hh"
#include "primitive.hh"
#include "shader.hh"
#include "resource_pool.hh"
#include "environment_map.hh"
#include "camera.hh"

namespace lt::method
{

skybox::skybox(
    render_target& target,
    resource_pool& pool,
    render_scene* scene
):  target_method(target),
    sky_shader(pool.get_shader(
        shader::path{"skybox.vert", "skybox.frag"}, {}
    )),
    scene(scene),
    skybox_sampler(pool.get_context(), GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE),
    quad(common::ensure_quad_primitive(pool))
{
    set_stencil_cull(0);
}

void skybox::set_scene(render_scene* s)
{
    this->scene = s;
}

render_scene* skybox::get_scene() const
{
    return scene;
}

void skybox::execute()
{
    target_method::execute();

    if(!scene) return;
    environment_map* skybox = scene->get_skybox();
    camera* cam = scene->get_camera();
    if(!skybox || !cam) return;

    glm::mat4 p = cam->get_projection();

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    stencil_cull();

    sky_shader->bind();

    sky_shader->set("skybox", skybox_sampler.bind(*skybox));
    sky_shader->set("exposure", skybox->get_exposure());
    sky_shader->set("inv_view", cam->get_global_transform());
    sky_shader->set("projection", glm::inverse(p));

    quad.draw();
}

std::string skybox::get_name() const
{
    return "skybox";
}

} // namespace lt::method
