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
#include "geometry_pass.hh"
#include "camera.hh"
#include "model.hh"
#include "object.hh"
#include "material.hh"
#include "primitive.hh"
#include "common_resources.hh"
#include "resource_pool.hh"
#include "multishader.hh"
#include "gbuffer.hh"
#include "shader_pool.hh"
#include "scene.hh"
#include "math.hh"
#include <utility>

namespace
{
    using namespace lt;
    void depth_pass(
        const shader::definition_map& common,
        multishader* geometry_shader,
        camera* cam,
        object_scene* s,
        vec3 ambient = vec3(0)
    ){
        glm::mat4 v = glm::inverse(cam->get_global_transform());
        glm::mat4 p = cam->get_projection();

        for(object* obj: s->get_objects())
        {
            const model* mod = obj->get_model();
            if(!mod) continue;

            glm::mat4 mv = v * obj->get_global_transform();
            glm::mat3 n_m(glm::inverseTranspose(mv));
            glm::mat4 mvp = p * mv;

            for(const model::vertex_group& group: *mod)
            {
                if(!group.mat || !group.mesh) continue;

                shader::definition_map definitions(common);
                group.mat->update_definitions(definitions);
                group.mesh->update_definitions(definitions);

                shader* s = geometry_shader->get(definitions);
                s->bind();

                s->set("mvp", mvp);
                s->set("m", mv);
                s->set("n_m", n_m);
                s->set("ambient", ambient);

                unsigned texture_index = 0;
                group.mat->apply(s, texture_index);
                group.mesh->draw();
            }
        }
    }
}

namespace lt::method
{

geometry_pass::geometry_pass(
    gbuffer& buf,
    resource_pool& pool,
    Scene scene,
    const options& opt
):  target_method(buf),
    scene_method(scene),
    options_method(opt),
    geometry_shader(pool.get_shader(
        shader::path{"generic.vert", "forward.frag"})
    )
{}

void geometry_pass::execute()
{
    target_method::execute();
    if(!geometry_shader || !has_all_scenes())
        return;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    stencil_draw();

    camera* cam = get_scene<camera_scene>()->get_camera();
    if(!cam) return;

    shader::definition_map common({
        {"OUTPUT_GEOMETRY", ""},
        {"OUTPUT_LIGHTING", ""},
        {"APPLY_EMISSION", ""}
    });

    if(opt.render_transparent)
    {
        common["MAX_ALPHA"] = "1.0f";
        common["MIN_ALPHA"] = "0.00390625f";
    }
    else common["MIN_ALPHA"] = "1.0f";

    if(opt.apply_ambient) common["APPLY_AMBIENT"];

    gbuffer* gbuf = static_cast<gbuffer*>(&get_target());

    if(opt.render_transparent)
    {
        // Draw depth first to extract top layer.
        gbuf->set_draw(gbuffer::DRAW_NONE);

        shader::definition_map depth_only({
            {"MAX_ALPHA", "1.0f"},
            {"MIN_ALPHA", "0.00390625f"}
        });

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        depth_pass(depth_only, geometry_shader, cam, get_scene<object_scene>());
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        gbuf->set_draw(gbuffer::DRAW_ALL);
        glEnable(GL_BLEND);
        // Don't blend geometry data channels.
        glBlendFunc(GL_ONE, GL_ZERO);

        // Transmittance
        glBlendFunci(
            gbuf->get_lighting_index(),
            GL_ZERO,
            GL_ONE_MINUS_SRC_ALPHA
        );
    }
    else gbuf->set_draw(gbuffer::DRAW_ALL);
    // Emission values are written to lighting during geometry pass, so DRAW_ALL
    // instead of DRAW_GEOMETRY.
    gbuf->update_definitions(common);

    depth_pass(
        common,
        geometry_shader,
        cam,
        get_scene<object_scene>(),
        get_scene<light_scene>()->get_ambient()
    );

    gbuf->set_draw(gbuffer::DRAW_LIGHTING);
}

} // namespace lt::method
