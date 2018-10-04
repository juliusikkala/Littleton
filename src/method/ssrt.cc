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
#include "ssrt.hh"
#include "shader.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include "resource_pool.hh"
#include "camera.hh"
#include "scene.hh"
#include "multishader.hh"
#include "common_resources.hh"
#include "environment_map.hh"
#include <stdexcept>

namespace lt::method
{

ssrt::ssrt(
    render_target& target,
    gbuffer& buf,
    resource_pool& pool,
    Scene scene,
    const options& opt
):  target_method(target), scene_method(scene), options_method(opt),
    stencil_handler(GL_EQUAL, 1, 1), buf(&buf), pool(pool),
    ssrt_shaders(pool.get_shader(shader::path{"fullscreen.vert", "ssrt.frag"})),
    blit_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "blit_texture.frag"}, {}
    )),
    quad(common::ensure_quad_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool)),
    mipmap_sampler(
        pool.get_context(),
        GL_NEAREST,
        GL_NEAREST_MIPMAP_NEAREST,
        GL_CLAMP_TO_EDGE
    ),
    cubemap_sampler(pool.get_context(), GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE)
{
    options_will_update(opt, true);
}

void ssrt::execute()
{
    const auto [
        roughness_cutoff,
        brdf_cutoff,
        max_steps,
        thickness,
        ray_offset,
        use_fallback_cubemap
    ] = opt;
    if(!ssrt_shader || !has_all_scenes() || !buf) return;

    camera* cam = get_scene<camera_scene>()->get_camera();
    if(!cam) return;

    texture* linear_depth = buf->get_linear_depth();
    texture* lighting = buf->get_lighting();
    texture* normal = buf->get_normal();
    texture* material = buf->get_material();
    texture* color = buf->get_color();
    if(!linear_depth || !lighting || !normal || !material || !color)
        return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    stencil_cull();

    glm::mat4 p = cam->get_projection();
    glm::uvec2 size(get_target().get_size());

    framebuffer_pool::loaner ssrt_buffer(pool.loan_framebuffer(
        size, {{GL_COLOR_ATTACHMENT0, {lighting->get_internal_format(), true}}}
    ));
    ssrt_buffer->bind();

    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);

    environment_map* skybox = get_scene<environment_scene>()->get_skybox();
    shader* s = use_fallback_cubemap && skybox ? ssrt_shader_env : ssrt_shader;

    s->bind();
    s->set("in_linear_depth", mipmap_sampler.bind(*linear_depth, 0));
    s->set("in_lighting", fb_sampler.bind(*lighting, 1));
    s->set("in_normal", fb_sampler.bind(*normal, 2));
    s->set("in_material", fb_sampler.bind(*material, 3));
    s->set("in_color", fb_sampler.bind(*color, 4));

    s->set("proj", p);
    s->set("projection_info", cam->get_projection_info());
    s->set("clip_info", cam->get_clip_info());
    s->set("near", -cam->get_near());

    s->set<int>("ssrt_ray_max_steps", max_steps);
    s->set("ssrt_thickness", thickness);
    s->set("ssrt_roughness_cutoff", roughness_cutoff);
    s->set("ssrt_brdf_cutoff", brdf_cutoff);
    s->set("ssrt_ray_offset", ray_offset);

    if(use_fallback_cubemap && skybox)
    {
        s->set("fallback_cubemap", true);
        s->set("ssrt_env_inv_view", cam->get_global_transform());
        s->set("ssrt_env", cubemap_sampler.bind(*skybox, 5));
        s->set("ssrt_env_exposure", skybox->get_exposure());
    }
    else
    {
        s->set("fallback_cubemap", false);
    }

    quad.draw();

    glEnable(GL_BLEND);
    get_target().bind();

    blit_shader->bind();
    blit_shader->set(
        "tex",
        fb_sampler.bind(*ssrt_buffer->get_texture_target(GL_COLOR_ATTACHMENT0))
    );

    quad.draw();
}

void ssrt::options_will_update(const options& next, bool initial)
{
    if(opt.thickness != next.thickness || initial)
    {
        texture* linear_depth = buf->get_linear_depth();

        // Thickness requires min-max depth buffer
        if(next.thickness > 0.0f && linear_depth &&
           linear_depth->get_external_format() != GL_RG) 
        {
            throw std::runtime_error(
                "Min-max (two channel) linear depth buffer required for SSRT with "
                "finite thickness"
            );
        }

        opt = next;

        refresh_shader();
    }
}

void ssrt::refresh_shader()
{
    shader::definition_map def = {
        {
            "RAY_MAX_LEVEL",
            std::to_string(calculate_mipmap_count(get_target().get_size())-1)
        }
    };
    if(opt.thickness < 0.0f) def["DEPTH_INFINITE_THICKNESS"];

    ssrt_shader = ssrt_shaders->get(def);
    def["FALLBACK_CUBEMAP"];
    ssrt_shader_env= ssrt_shaders->get(def);
}

} // namespace lt::method
