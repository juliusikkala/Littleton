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
#include "context.hh"
#include "multishader.hh"
#include "shader.hh"
#include "math.hh"
#include "gbuffer.hh"
#include "resource_pool.hh"
#include "camera.hh"
#include "common_resources.hh"
#include "spherical_gaussians.hh"
#include <algorithm>

namespace lt::method
{

apply_sg::apply_sg(
    render_target& target,
    gbuffer& buf,
    resource_pool& pool,
    Scene scene,
    const options& opt
):  target_method(target),
    scene_method(scene),
    options_method(opt),
    glresource(target.get_context()),
    stencil_handler(GL_NOTEQUAL, 1<<7, 1<<7),
    buf(&buf),
    sg_shader(pool.get_shader(shader::path{"generic.vert", "sg/apply.frag"})),
    cube(common::ensure_cube_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool)),
    linear_sampler(common::ensure_linear_sampler(pool))
{
}

void apply_sg::execute()
{
    target_method::execute();
    const auto [min_specular_roughness] = opt;

    if(!sg_shader || !has_all_scenes())
        return;

    camera* cam = get_scene<camera_scene>()->get_camera();
    if(!cam) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // Draw back faces only of the bounding cube
    glFrontFace(GL_CW);

    if(&get_target() == buf && buf->get_indirect_lighting() != nullptr)
        buf->set_draw(gbuffer::DRAW_INDIRECT_LIGHTING);

    glm::mat4 p = cam->get_projection();
    glm::mat4 v = glm::inverse(cam->get_global_transform());
    glm::mat4 vp = p * v;

    std::vector<sg_group*> groups_by_density(
        get_scene<sg_scene>()->get_sg_groups()
    );
    std::sort(
        groups_by_density.begin(),
        groups_by_density.end(),
        [](sg_group* a, sg_group* b){
            return a->get_density() > b->get_density();
        }
    );

    unsigned bind_index = 0;
    buf->bind_textures(fb_sampler, bind_index);
    unsigned available_texture_slots =
        get_context()[GL_MAX_TEXTURE_IMAGE_UNITS] - bind_index;

    shader* s = sg_shader->get({
        {"MAX_LOBE_COUNT", std::to_string(available_texture_slots)}
    });
    s->set("projection_info", cam->get_projection_info());
    s->set("clip_info", cam->get_clip_info());
    s->set("min_specular_roughness", min_specular_roughness);

    bind_index = 0;
    buf->set_uniforms(s, bind_index);

    std::vector<vec3> axis_buffer(available_texture_slots);
    std::vector<float> sharpness_buffer(available_texture_slots);
    std::vector<int> amplitude_buffer(available_texture_slots);

    for(sg_group* group: groups_by_density)
    {
        const std::vector<sg_lobe>& lobes = group->get_lobes();

        glm::mat4 m = group->get_global_transform();
        glm::mat4 mv = v * m;

        s->set("m", glm::mat4(1.0));
        s->set("inv_mv", glm::inverse(mv));
        s->set("mvp", vp * m);

        stencil_cull();
        unsigned i = 0;
        while(i < lobes.size())
        {
            unsigned batch_size = min(
                (unsigned)lobes.size() - i,
                available_texture_slots
            );
            s->set<int>("sg_lobe_count", batch_size);

            unsigned j = 0;
            for(j = 0; j < batch_size; ++j)
            {
                std::string str_index = std::to_string(j);
                amplitude_buffer[j] = linear_sampler.bind(
                    group->get_amplitudes(i+j),
                    bind_index + j
                );
                axis_buffer[j] = lobes[i + j].axis;
                sharpness_buffer[j] = lobes[i + j].sharpness;
            }

            for(unsigned k = j; k < available_texture_slots; ++k)
                amplitude_buffer[k] = amplitude_buffer[j - 1];

            s->set(
                "sg_amplitude",
                amplitude_buffer.size(),
                amplitude_buffer.data()
            );
            s->set("sg_axis", axis_buffer.size(), axis_buffer.data());
            s->set("sg_sharpness",
                sharpness_buffer.size(),
                sharpness_buffer.data()
            );

            i += available_texture_slots;
            // If last iteration, draw to stencil
            if(i >= lobes.size())
                stencil_draw_cull();

            cube.draw();
        }
    }

    if(&get_target() == buf) buf->set_draw(gbuffer::DRAW_LIGHTING);

    glFrontFace(GL_CCW);
}

} // namespace lt::method
