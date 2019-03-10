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
    Scene scene
):  target_method(target),
    scene_method(scene),
    stencil_handler(GL_NOTEQUAL, 1, 1),
    sky_shader(pool.get_shader(
        shader::path{"cast_ray.vert", "skybox.frag"}, {}
    )),
    cubemap_sky_shader(pool.get_shader(
        shader::path{"cast_ray.vert", "skybox.frag", "cast_ray.geom"},
        {{"CUBEMAP", ""}}
    )),
    skybox_sampler(
        pool.get_context(),
        interpolation::LINEAR,
        interpolation::LINEAR,
        GL_CLAMP_TO_EDGE
    ),
    quad(common::ensure_quad_primitive(pool))
{
}

void skybox::execute()
{
    target_method::execute();

    if(!has_all_scenes()) return;

    camera_scene* cs = get_scene<camera_scene>();
    environment_map* skybox = get_scene<environment_scene>()->get_skybox();
    camera* cam = cs->get_camera();
    if(!skybox || !cam) return;

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    stencil_cull();

    GLenum target = get_target().get_target();
    bool cubemap =
        target == GL_TEXTURE_CUBE_MAP || target == GL_TEXTURE_CUBE_MAP_ARRAY;

    if(cubemap)
    {
        cubemap_sky_shader->bind();
        cubemap_sky_shader->set("skybox", skybox_sampler.bind(*skybox));
        cubemap_sky_shader->set("exposure", skybox->get_exposure());

        unsigned layers = min(
            (unsigned)cs->camera_count(), get_target().get_dimensions().z
        );

        const std::vector<camera*> cameras = cs->get_cameras();
        std::vector<glm::mat4> face_ivps;
        face_ivps.reserve(layers*6);
        for(unsigned layer = 0; layer < layers; ++layer)
            for(unsigned face = 0; face < 6; ++face)
                face_ivps.push_back(
                    cameras[layer]->get_inverse_orientation_projection(face)
                );

        for(unsigned i = 0; i < layers; ++i)
        {
            cubemap_sky_shader->set("face_ivps", 6, face_ivps.data() + i*6);
            cubemap_sky_shader->set("begin_layer_face", (int)i*6);
            quad.draw();
        }
    }
    else
    {
        glm::mat4 p = cam->get_projection();
        sky_shader->bind();

        sky_shader->set("skybox", skybox_sampler.bind(*skybox));
        sky_shader->set("exposure", skybox->get_exposure());
        sky_shader->set("ivp", toMat4(cam->get_global_orientation()) * glm::inverse(p));

        quad.draw();
    }
}

} // namespace lt::method
