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
    render_scene* scene
):  target_method(target), stencil_handler(GL_EQUAL, 1, 1),
    buf(&buf), pool(pool),
    ssrt_shaders(pool.get_shader(shader::path{"fullscreen.vert", "ssrt.frag"})),
    blit_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "blit_texture.frag"}, {}
    )),
    scene(scene),
    quad(common::ensure_quad_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool)),
    mipmap_sampler(
        pool.get_context(),
        GL_NEAREST,
        GL_NEAREST_MIPMAP_NEAREST,
        GL_CLAMP_TO_EDGE
    ),
    cubemap_sampler(pool.get_context(), GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE),
    max_steps(500),
    thickness(-1.0f),
    roughness_cutoff(0.5f),
    brdf_cutoff(0.0f),
    ray_offset(0.01f),
    fallback_cubemap(false)
{
    set_thickness();
}

void ssrt::set_scene(render_scene* scene)
{
    this->scene = scene;
}

render_scene* ssrt::get_scene() const
{
    return scene;
}

void ssrt::set_max_steps(unsigned max_steps)
{
    this->max_steps = max_steps;
}

void ssrt::set_roughness_cutoff(float cutoff)
{
    roughness_cutoff = cutoff;
}

void ssrt::set_brdf_cutoff(float cutoff)
{
    brdf_cutoff = cutoff;
}

void ssrt::set_thickness(float thickness)
{
    this->thickness = thickness;
    texture* linear_depth = buf->get_linear_depth();

    // Thickness requires min-max depth buffer
    if(thickness > 0.0f && linear_depth &&
       linear_depth->get_external_format() != GL_RG) 
    {
        throw std::runtime_error(
            "Min-max (two channel) linear depth buffer required for SSRT with "
            "finite thickness"
        );
    }

    refresh_shader();
}

void ssrt::set_ray_offset(float offset)
{
    ray_offset = offset;
}

void ssrt::use_fallback_cubemap(bool use)
{
    this->fallback_cubemap = use;
}

void ssrt::execute()
{
    if(!ssrt_shader || !scene || !buf) return;

    camera* cam = scene->get_camera();
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

    environment_map* skybox = scene->get_skybox();
    shader* s = fallback_cubemap && skybox ? ssrt_shader_env : ssrt_shader;

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

    s->set<int>("ray_max_steps", max_steps);
    s->set("thickness", thickness);
    s->set("roughness_cutoff", roughness_cutoff);
    s->set("brdf_cutoff", brdf_cutoff);
    s->set("ray_offset", ray_offset);

    if(fallback_cubemap && skybox)
    {
        s->set("fallback_cubemap", true);
        s->set("inv_view", cam->get_global_transform());
        s->set("env", cubemap_sampler.bind(*skybox, 5));
        s->set("exposure", skybox->get_exposure());
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

std::string ssrt::get_name() const
{
    return "ssrt";
}

void ssrt::refresh_shader()
{
    shader::definition_map def = {
        {
            "RAY_MAX_LEVEL",
            std::to_string(calculate_mipmap_count(get_target().get_size())-1)
        }
    };
    if(thickness < 0.0f) def["DEPTH_INFINITE_THICKNESS"];

    ssrt_shader = ssrt_shaders->get(def);
    def["FALLBACK_CUBEMAP"];
    ssrt_shader_env= ssrt_shaders->get(def);
}

} // namespace lt::method
