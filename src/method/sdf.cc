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
#include "environment_map.hh"

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
    pool(pool),
    sdf_shader(pool.get_shader(shader::path{"cast_ray.vert", "sdf.frag"})),
    fb_sampler(common::ensure_framebuffer_sampler(pool)),
        mipmap_sampler(
            pool.get_context(),
            GL_NEAREST,
            GL_NEAREST_MIPMAP_NEAREST,
            GL_CLAMP_TO_EDGE
    ),
    cubemap_sampler(pool.get_context(), GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE),
    quad(common::ensure_quad_primitive(pool))
{
}

void render_sdf::execute()
{
    target_method::execute();
    if(!sdf_shader || !has_all_scenes())
        return;

    auto [
        apply_ambient, apply_lighting, render_transparent, write_depth,
        num_refractions, num_reflections, max_steps, min_dist, max_dist,
        step_ratio, hit_ratio, use_ssrt, ssrt_roughness_cutoff,
        ssrt_brdf_cutoff, ssrt_max_steps, ssrt_thickness
    ] = opt;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    stencil_draw();

    light_scene* lights = get_scene<light_scene>();
    sdf_scene* sdfs = get_scene<sdf_scene>();
    camera* cam = get_scene<camera_scene>()->get_camera();
    if(!cam) return;

    glm::mat4 p = cam->get_projection();
    glm::mat4 ip = glm::inverse(cam->get_projection());

    gbuffer* gbuf = static_cast<gbuffer*>(&get_target());
    glm::uvec2 size(get_target().get_size());

    texture* linear_depth = gbuf->get_linear_depth();
    texture* lighting = gbuf->get_lighting();
    if(!linear_depth || !lighting) return;

    framebuffer_pool::loaner ssrt_buffer;

    // Copy necessary buffers for SSRT
    if(use_ssrt)
    {
        ssrt_buffer = pool.loan_framebuffer(
            size, {
                {GL_COLOR_ATTACHMENT0, {lighting->get_internal_format(), true}}
            }
        );
        lighting = ssrt_buffer->get_texture_target(GL_COLOR_ATTACHMENT0);

        gbuf->set_draw(gbuffer::DRAW_LIGHTING);

        ssrt_buffer->bind(GL_DRAW_FRAMEBUFFER);
        gbuf->bind(GL_READ_FRAMEBUFFER);
        
        /* Copy lighting */
        glBlitFramebuffer(
            0, 0, size.x, size.y,
            0, 0, size.x, size.y,
            GL_COLOR_BUFFER_BIT,
            GL_NEAREST
        );
    }

    gbuf->bind();
    gbuf->set_draw(gbuffer::DRAW_ALL_EXCEPT_LINEAR_DEPTH);

    if(!write_depth) glDepthMask(GL_FALSE);

    shader* s = NULL;
    uint64_t hash = sdfs->get_hash();
    auto it = cached.find(hash);
    
    if(it != cached.end()) s = it->second;
    else {
        shader::definition_map def({
            {"LOCAL_VIEW_DIR", ""},
            {"VERTEX_NORMAL", ""},
            {"FALLBACK_CUBEMAP", ""}
        });

        if(use_ssrt)
        {
            def["USE_SSRT"];
            def["RAY_MAX_LEVEL"] = std::to_string(
                calculate_mipmap_count(get_target().get_size())-1
            );
            if(ssrt_thickness < 0.0f) def["DEPTH_INFINITE_THICKNESS"];
        }
    
        sdfs->update_definitions(def);

        if(apply_ambient) def["APPLY_AMBIENT"];
        gbuf->update_definitions(def);

        s = sdf_shader->get(def);
        cached[hash] = s;
    }
    s->bind();

    environment_map* skybox = get_scene<environment_scene>()->get_skybox();

    if(use_ssrt)
    {
        if(skybox)
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

        s->set("in_linear_depth", mipmap_sampler.bind(*linear_depth, 0));
        s->set("in_lighting", fb_sampler.bind(*lighting, 1));
        s->set<int>("ssrt_ray_max_steps", ssrt_max_steps);
        s->set("ssrt_thickness", ssrt_thickness);
        s->set("ssrt_roughness_cutoff", ssrt_roughness_cutoff);
        s->set("ssrt_brdf_cutoff", ssrt_brdf_cutoff);
        s->set("ssrt_ray_offset", 0.0f);
    }

    s->set("ambient", lights->get_ambient());
    s->set("ivp", toMat4(cam->get_global_orientation()) * ip);
    s->set("ip", ip);
    s->set("proj", p);
    s->set("projection_info", cam->get_projection_info());
    s->set("clip_info", cam->get_clip_info());
    s->set("camera_pos", cam->get_global_position());
    s->set("near", -cam->get_near());
    s->set("n_v", glm::mat3(glm::transpose(cam->get_global_transform())));
    s->set("view", glm::inverse(cam->get_global_transform()));
    s->set<int>("num_refractions", num_refractions);
    s->set<int>("num_reflections", num_reflections);
    s->set<int>("max_steps", max_steps);
    s->set("min_dist", min_dist);
    s->set("max_dist", max_dist);
    s->set("step_ratio", step_ratio);
    s->set("hit_ratio", hit_ratio);
    s->set<float>("time", get_time_sec());

    quad.draw();

    if(!write_depth) glDepthMask(GL_TRUE);
    gbuf->set_draw(gbuffer::DRAW_LIGHTING);
}

void render_sdf::options_will_update(const options&)
{
    cached.clear();
}

}
