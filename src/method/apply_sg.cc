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
#include "apply_sg.hh"
#include "shader.hh"
#include "math.hh"
#include "gbuffer.hh"
#include "resource_pool.hh"
#include "camera.hh"
#include "scene.hh"
#include "common_resources.hh"
#include "spherical_gaussians.hh"
#include <algorithm>

namespace lt::method
{

apply_sg::apply_sg(
    render_target& target,
    gbuffer& buf,
    resource_pool& pool,
    render_scene* scene
):  target_method(target),
    stencil_handler(GL_NOTEQUAL, 1<<7, 1<<7),
    buf(&buf),
    sg_shader(pool.get_shader(
        shader::path{"generic.vert", "sao/apply_sg.frag"}, {}
    )),
    scene(scene),
    cube(common::ensure_cube_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool)),
    linear_sampler(common::ensure_linear_sampler(pool))
{
}

void apply_sg::set_scene(render_scene* scene)
{
    this->scene = scene;
}

render_scene* apply_sg::get_scene() const
{
    return scene;
}

void apply_sg::execute()
{
    target_method::execute();

    if(!sg_shader || !scene)
        return;

    camera* cam = scene->get_camera();
    if(!cam) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glm::mat4 p = cam->get_projection();
    glm::mat4 v = glm::inverse(cam->get_global_transform());
    glm::mat4 vp = p * v;

    std::vector<sg_group*> groups_by_density(scene->get_sg_groups());
    std::sort(
        groups_by_density.begin(),
        groups_by_density.end(),
        [](sg_group* a, sg_group* b){
            return a->get_density() > b->get_density();
        }
    );

    unsigned bind_index = 0;
    buf->bind_textures(fb_sampler, bind_index);
    bind_index = 0;
    buf->set_uniforms(sg_shader, bind_index);
    sg_shader->set("projection_info", cam->get_projection_info());
    sg_shader->set("clip_info", cam->get_clip_info());

    for(sg_group* group: groups_by_density)
    {
        const std::vector<sg_lobe>& lobes = group->get_lobes();

        glm::mat4 m = group->get_global_transform();
        glm::mat4 mv = v * m;

        sg_shader->set("m", glm::mat4(1.0));
        sg_shader->set("inv_m", glm::inverseTranspose(mv));
        sg_shader->set("mvp", vp * m);

        stencil_cull();
        for(unsigned i = 0; i < lobes.size(); ++i)
        {
            if(i == lobes.size()-1)
                stencil_draw_cull();
            sg_shader->set(
                "sg_lobe",
                linear_sampler.bind(group->get_amplitudes(i), 6)
            );
            cube.draw();
        }
    }
}

std::string apply_sg::get_name() const 
{
    return "apply_sg";
}

} // namespace lt::method
