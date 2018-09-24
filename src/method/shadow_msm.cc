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
#include "shadow_msm.hh"
#include "resource_pool.hh"
#include "helpers.hh"
#include "object.hh"
#include "camera.hh"
#include "scene.hh"
#include "common_resources.hh"

namespace
{
using namespace lt;
using namespace lt::method;

template<typename L>
void render_single(
    L* msm,
    resource_pool& pool,
    object_scene* objects,
    const primitive& quad,
    shader* depth_shader,
    shader* horizontal_blur_shader,
    shader* vertical_blur_shader,
    sampler& moment_sampler
){
    texture& moments = msm->get_moments();
    framebuffer& moments_buffer = msm->get_framebuffer();

    render_target* target = nullptr;

    framebuffer_pool::loaner postprocess_buffer(pool.loan_framebuffer(
        moments.get_size(),
        {{GL_DEPTH_ATTACHMENT, {GL_DEPTH_COMPONENT16}},
         {GL_COLOR_ATTACHMENT0, {GL_RGBA16, true}}},
        0
    ));
    framebuffer_pool::loaner multisample_buffer(nullptr);
    if(msm->get_samples() != 0)
    {
        multisample_buffer = pool.loan_framebuffer(
            moments.get_size(),
            {{GL_DEPTH_ATTACHMENT, {GL_DEPTH_COMPONENT16}},
             {GL_COLOR_ATTACHMENT0, {GL_RGBA16}}},
            msm->get_samples()
        );
        target = multisample_buffer.get();
    }
    else
    {
        target = postprocess_buffer.get();
    }

    // Render depth data
    glEnable(GL_DEPTH_TEST);

    target->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 vp = msm->get_projection() * msm->get_view();

    for(object* obj: objects->get_objects())
    {
        const model* mod = obj->get_model();
        if(!mod) continue;

        glm::mat4 m = obj->get_global_transform();
        glm::mat4 mvp = vp * m;
        depth_shader->set("m", m);
        depth_shader->set("mvp", mvp);

        for(const model::vertex_group& group: *mod)
        {
            if(!group.mesh) continue;

            group.mesh->draw();
        }
    }

    target->bind(GL_READ_FRAMEBUFFER);
    moments_buffer.bind(GL_DRAW_FRAMEBUFFER);

    glm::uvec2 target_size = moments.get_size();
    glBlitFramebuffer(
        0, 0, target_size.x, target_size.y,
        0, 0, target_size.x, target_size.y,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST
    );

    // Blur the depth (thanks to moment magic this can be done!)
    unsigned radius = msm->get_radius();
    if(radius == 0) return;

    vertical_blur_shader->bind();

    glDisable(GL_DEPTH_TEST);

    postprocess_buffer->bind();
    vertical_blur_shader->set(
        "tex",
        moment_sampler.bind(moments, 0)
    );
    vertical_blur_shader->set("samples", (int)(2 * radius + 1));
    quad.draw();

    moments_buffer.bind();
    horizontal_blur_shader->set(
        "tex",
        moment_sampler.bind(
            *postprocess_buffer->get_texture_target(GL_COLOR_ATTACHMENT0), 0
        )
    );
    horizontal_blur_shader->set("samples", (int)(2 * radius + 1));
    quad.draw();
}

}

namespace lt::method
{

shadow_msm::shadow_msm(resource_pool& pool, Scene scene)
:   glresource(pool.get_context()),
    shadow_method(scene),
    pool(pool),
    depth_shader(pool.get_shader(
        shader::path{"generic.vert", "shadow/directional_msm.frag"},
        {{"VERTEX_POSITION", "0"},
         {"DISCARD_ALPHA", "0.5"}}
    )),
    cubemap_depth_shader(pool.get_shader(
        shader::path{"generic.vert", "shadow/omni_msm.frag", "cubemap.geom"},
        {{"VERTEX_POSITION", "0"},
         {"DISCARD_ALPHA", "0.5"}}
    )),
    perspective_depth_shader(pool.get_shader(
        shader::path{"generic.vert", "shadow/omni_msm.frag"},
        {{"VERTEX_POSITION", "0"},
         {"DISCARD_ALPHA", "0.5"}}
    )),
    vertical_blur_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "blur.frag"}, {{"VERTICAL", ""}}
    )),
    horizontal_blur_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "blur.frag"}, {{"HORIZONTAL", ""}}
    )),
    quad(common::ensure_quad_primitive(pool)),
    moment_sampler(
       pool.get_context(),
       GL_LINEAR,
       GL_LINEAR,
       GL_CLAMP_TO_EDGE
    ),
    cubemap_moment_sampler(
        pool.get_context(),
        GL_LINEAR,
        GL_LINEAR,
        GL_CLAMP_TO_EDGE
    )
{}

shader::definition_map shadow_msm::get_directional_definitions() const
{
    return {
        {"SHADOW_MAPPING", "shadow/directional_msm.glsl"},
        {"DIRECTIONAL_SHADOW_MAPPING", ""}
    };
}

shader::definition_map shadow_msm::get_omni_definitions() const
{
    return {
        {"SHADOW_MAPPING", "shadow/omni_msm.glsl"},
        {"OMNI_SHADOW_MAPPING", ""}
    };
}

shader::definition_map shadow_msm::get_perspective_definitions() const
{
    return {
        {"SHADOW_MAPPING", "shadow/perspective_msm.glsl"},
        {"PERSPECTIVE_SHADOW_MAPPING", ""}
    };
}

void shadow_msm::set_shadow_map_uniforms(
    shader* s,
    unsigned& texture_index,
    directional_shadow_map* shadow_map,
    const std::string& prefix,
    const glm::mat4& pos_to_world
){
    directional_shadow_map_msm* sm =
        static_cast<directional_shadow_map_msm*>(shadow_map);

    glm::mat4 lvp = sm->get_projection() * sm->get_view();

    s->set(
        prefix + "map",
        moment_sampler.bind(sm->moments, texture_index++)
    );
    s->set(prefix + "mvp", lvp * pos_to_world);
}

void shadow_msm::set_shadow_map_uniforms(
    shader* s,
    unsigned& texture_index,
    omni_shadow_map* shadow_map,
    const std::string& prefix,
    const glm::mat4& pos_to_world
){
    omni_shadow_map_msm* sm = static_cast<omni_shadow_map_msm*>(shadow_map);

    s->set(
        prefix + "map",
        cubemap_moment_sampler.bind(sm->moments, texture_index++)
    );
    s->set(prefix + "far_plane", sm->get_range().y);
}

void shadow_msm::set_shadow_map_uniforms(
    shader* s,
    unsigned& texture_index,
    perspective_shadow_map* shadow_map,
    const std::string& prefix,
    const glm::mat4& pos_to_world
){
    perspective_shadow_map_msm* sm =
        static_cast<perspective_shadow_map_msm*>(shadow_map);

    glm::mat4 lvp = sm->get_projection() * sm->get_view();

    s->set(
        prefix + "map",
        moment_sampler.bind(sm->moments, texture_index++)
    );
    s->set(prefix + "mvp", lvp * pos_to_world);
    s->set(prefix + "far_plane", sm->get_range().y);
}

void shadow_msm::execute()
{
    if(!has_all_scenes()) return;

    shadow_scene* shadows = get_scene<shadow_scene>();
    object_scene* objects = get_scene<object_scene>();

    const std::vector<directional_shadow_map*>* directional_shadow_maps = NULL;
    {
        const shadow_scene::directional_map& directional =
            shadows->get_directional_shadows();
        auto it = directional.find(this);
        if(it != directional.end()) directional_shadow_maps = &it->second;
    }

    const std::vector<omni_shadow_map*>* omni_shadow_maps = NULL;
    {
        const shadow_scene::omni_map& omni = shadows->get_omni_shadows();
        auto it = omni.find(this);
        if(it != omni.end()) omni_shadow_maps = &it->second;
    }

    const std::vector<perspective_shadow_map*>* perspective_shadow_maps = NULL;
    {
        const shadow_scene::perspective_map& perspective =
            shadows->get_perspective_shadows();
        auto it = perspective.find(this);
        if(it != perspective.end()) perspective_shadow_maps = &it->second;
    }

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glClearColor(0.0f, 0.63f, 0.0f, 0.63f);

    if(directional_shadow_maps)
    {
        depth_shader->bind();
        //TODO: Handle transparency correctly by setting the material.
        depth_shader->set("input_material.color_factor", glm::vec4(1.0f));
        for(directional_shadow_map* sm: *directional_shadow_maps)
        {
            directional_shadow_map_msm* msm =
                static_cast<directional_shadow_map_msm*>(sm);
            render_single(
                msm,
                pool,
                objects,
                quad,
                depth_shader,
                horizontal_blur_shader,
                vertical_blur_shader,
                moment_sampler
            );
        }
    }

    if(perspective_shadow_maps)
    {
        perspective_depth_shader->bind();
        //TODO: Handle transparency correctly by setting the material.
        perspective_depth_shader->set(
            "input_material.color_factor",
            glm::vec4(1.0f)
        );
        for(perspective_shadow_map* sm: *perspective_shadow_maps)
        {
            perspective_shadow_map_msm* msm =
                static_cast<perspective_shadow_map_msm*>(sm);
            perspective_depth_shader->set("far_plane", msm->get_range().y);
            perspective_depth_shader->set(
                "pos", msm->get_light()->get_global_position()
            );
            render_single(
                msm,
                pool,
                objects,
                quad,
                perspective_depth_shader,
                horizontal_blur_shader,
                vertical_blur_shader,
                moment_sampler
            );
        }
    }

    if(omni_shadow_maps)
    {
        glEnable(GL_DEPTH_TEST);

        cubemap_depth_shader->bind();
        //TODO: Handle transparency correctly by setting the material.
        cubemap_depth_shader->set(
            "input_material.color_factor",
            glm::vec4(1.0f)
        );

        for(omni_shadow_map* sm: *omni_shadow_maps)
        {
            omni_shadow_map_msm* msm = static_cast<omni_shadow_map_msm*>(sm);
            // Render depth data

            msm->moments_buffer.bind();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 proj = msm->get_projection();

            glm::mat4 face_vps[6] = {
                proj * msm->get_view(0), proj * msm->get_view(1),
                proj * msm->get_view(2), proj * msm->get_view(3),
                proj * msm->get_view(4), proj * msm->get_view(5)
            };
            cubemap_depth_shader->set("face_vps", 6, face_vps);
            cubemap_depth_shader->set(
                "pos", msm->get_light()->get_global_position()
            );
            cubemap_depth_shader->set("far_plane", msm->get_range().y);

            for(object* obj: objects->get_objects())
            {
                const model* mod = obj->get_model();
                if(!mod) continue;

                glm::mat4 m = obj->get_global_transform();
                cubemap_depth_shader->set("m", m);
                cubemap_depth_shader->set("mvp", m);

                for(const model::vertex_group& group: *mod)
                {
                    if(!group.mesh) continue;

                    group.mesh->draw();
                }
            }
        }
    }
}

} // namespace lt::method

namespace lt
{

directional_shadow_map_msm::directional_shadow_map_msm(
    method::shadow_msm* method,
    context& ctx,
    glm::uvec2 size,
    unsigned samples,
    unsigned radius,
    glm::vec3 offset,
    glm::vec2 area,
    glm::vec2 depth_range,
    directional_light* light
):  directional_shadow_map(method, offset, area, depth_range, light),
    moments(ctx, size, GL_RGBA16, GL_FLOAT),
    moments_buffer(ctx, size, {{GL_COLOR_ATTACHMENT0, {&moments}}}),
    samples(samples),
    radius(radius)
{
}

directional_shadow_map_msm::directional_shadow_map_msm(
    directional_shadow_map_msm&& other
):  directional_shadow_map(other),
    moments(std::move(other.moments)),
    moments_buffer(std::move(other.moments_buffer)),
    samples(other.samples),
    radius(other.radius)
{
}

unsigned directional_shadow_map_msm::get_samples() const
{
    return samples;
}

void directional_shadow_map_msm::set_radius(unsigned radius)
{
    this->radius = radius;
}

unsigned directional_shadow_map_msm::get_radius() const
{
    return radius;
}

texture& directional_shadow_map_msm::get_moments()
{
    return moments;
}

const texture& directional_shadow_map_msm::get_moments() const
{
    return moments;
}

framebuffer& directional_shadow_map_msm::get_framebuffer()
{
    return moments_buffer;
}

const framebuffer& directional_shadow_map_msm::get_framebuffer() const
{
    return moments_buffer;
}

omni_shadow_map_msm::omni_shadow_map_msm(
    method::shadow_msm* method,
    context& ctx,
    glm::uvec2 size,
    unsigned samples,
    float radius,
    glm::vec2 depth_range,
    point_light* light
):  omni_shadow_map(method, depth_range, light),
    moments(ctx, size, GL_RGBA16, GL_FLOAT, 0, GL_TEXTURE_CUBE_MAP),
    moments_buffer(
        ctx,
        size,
        {{GL_COLOR_ATTACHMENT0, {&moments}},
         {GL_DEPTH_ATTACHMENT, {GL_DEPTH_COMPONENT16}}},
        0, GL_TEXTURE_CUBE_MAP
    ),
    samples(samples)
{
}

omni_shadow_map_msm::omni_shadow_map_msm(omni_shadow_map_msm&& other)
:   omni_shadow_map(other),
    moments(std::move(other.moments)),
    moments_buffer(std::move(other.moments_buffer)),
    samples(other.samples)
{
}

unsigned omni_shadow_map_msm::get_samples() const
{
    return samples;
}

texture& omni_shadow_map_msm::get_moments()
{
    return moments;
}

const texture& omni_shadow_map_msm::get_moments() const
{
    return moments;
}

framebuffer& omni_shadow_map_msm::get_framebuffer()
{
    return moments_buffer;
}

const framebuffer& omni_shadow_map_msm::get_framebuffer() const
{
    return moments_buffer;
}

perspective_shadow_map_msm::perspective_shadow_map_msm(
    method::shadow_msm* method,
    context& ctx,
    glm::uvec2 size,
    unsigned samples,
    unsigned radius,
    double fov,
    glm::vec2 depth_range,
    point_light* light
):  perspective_shadow_map(method, fov, depth_range, light),
    moments(ctx, size, GL_RGBA16, GL_FLOAT),
    moments_buffer(ctx, size, {{GL_COLOR_ATTACHMENT0, {&moments}}}),
    samples(samples), radius(radius)
{}

perspective_shadow_map_msm::perspective_shadow_map_msm(
    perspective_shadow_map_msm&& other
):  perspective_shadow_map(other), moments(std::move(other.moments)),
    moments_buffer(std::move(other.moments_buffer)), samples(other.samples),
    radius(other.radius)
{
}

unsigned perspective_shadow_map_msm::get_samples() const
{
    return samples;
}

void perspective_shadow_map_msm::set_radius(unsigned radius)
{
    this->radius = radius;
}

unsigned perspective_shadow_map_msm::get_radius() const
{
    return radius;
}

texture& perspective_shadow_map_msm::get_moments()
{
    return moments;
}

const texture& perspective_shadow_map_msm::get_moments() const
{
    return moments;
}

framebuffer& perspective_shadow_map_msm::get_framebuffer()
{
    return moments_buffer;
}

const framebuffer& perspective_shadow_map_msm::get_framebuffer() const
{
    return moments_buffer;
}

} // namespace lt

