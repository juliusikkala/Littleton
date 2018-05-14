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
#include "shadow_pcf.hh"
#include "resource_pool.hh"
#include "helpers.hh"
#include "object.hh"
#include "camera.hh"
#include "scene.hh"
#include "common_resources.hh"

namespace lt::method
{

shadow_pcf::shadow_pcf(resource_pool& pool, render_scene* scene)
:   shadow_method(scene),
    depth_shader(pool.get_shader(
        shader::path{"generic.vert", "empty.frag"},
        {{"VERTEX_POSITION", "0"},
         {"DISCARD_ALPHA", "0.5"}}
    )),
    cubemap_depth_shader(pool.get_shader(
        shader::path{"generic.vert", "shadow/omni_pcf.frag", "cubemap.geom"},
        {{"VERTEX_POSITION", "0"},
         {"DISCARD_ALPHA", "0.5"}}
    )),
    perspective_depth_shader(pool.get_shader(
        shader::path{"generic.vert", "shadow/omni_pcf.frag"},
        {{"VERTEX_POSITION", "0"},
         {"DISCARD_ALPHA", "0.5"}}
    )),
    shadow_noise_2d(
        common::ensure_circular_random_texture(pool, glm::uvec2(512))
    ),
    shadow_noise_3d(
        common::ensure_spherical_random_texture(pool, glm::uvec2(512))
    ),
    kernel(common::ensure_circular_poisson_texture(pool, 256)),
    shadow_sampler(
        pool.get_context(),
        GL_LINEAR,
        GL_LINEAR,
        GL_CLAMP_TO_BORDER,
        0,
        glm::vec4(1),
        GL_COMPARE_REF_TO_TEXTURE
    ),
    cubemap_shadow_sampler(
        pool.get_context(),
        GL_NEAREST,
        GL_NEAREST,
        GL_CLAMP_TO_EDGE,
        0,
        glm::vec4(1),
        GL_COMPARE_REF_TO_TEXTURE
    ),
    noise_sampler(pool.get_context(), GL_NEAREST, GL_NEAREST, GL_REPEAT, 0)
{
    cubemap_depth_shader->load();
}

void shadow_pcf::set_directional_uniforms(
    shader* s,
    unsigned& texture_index
){
    s->set(
        "shadow_noise",
        noise_sampler.bind(shadow_noise_2d, texture_index++)
    );
    s->set("shadow_kernel", noise_sampler.bind(kernel, texture_index++));
}

void shadow_pcf::set_omni_uniforms(
    shader* s,
    unsigned& texture_index
){
    s->set(
        "shadow_noise",
        noise_sampler.bind(shadow_noise_3d, texture_index++)
    );
    s->set("shadow_kernel", noise_sampler.bind(kernel, texture_index++));
}

void shadow_pcf::set_perspective_uniforms(
    shader* s,
    unsigned& texture_index
){
    s->set(
        "shadow_noise",
        noise_sampler.bind(shadow_noise_2d, texture_index++)
    );
    s->set("shadow_kernel", noise_sampler.bind(kernel, texture_index++));
}

shader::definition_map shadow_pcf::get_directional_definitions() const
{
    return {
        {"SHADOW_MAPPING", "shadow/directional_pcf.glsl"},
        {"DIRECTIONAL_SHADOW_MAPPING", ""}
    };
}

shader::definition_map shadow_pcf::get_omni_definitions() const
{
    return {
        {"SHADOW_MAPPING", "shadow/omni_pcf.glsl"},
        {"OMNI_SHADOW_MAPPING", ""}
    };
}

shader::definition_map shadow_pcf::get_perspective_definitions() const
{
    return {
        {"SHADOW_MAPPING", "shadow/perspective_pcf.glsl"},
        {"PERSPECTIVE_SHADOW_MAPPING", ""}
    };
}

void shadow_pcf::set_shadow_map_uniforms(
    shader* s,
    unsigned& texture_index,
    directional_shadow_map* shadow_map,
    const std::string& prefix,
    const glm::mat4& pos_to_world
){
    directional_shadow_map_pcf* sm =
        static_cast<directional_shadow_map_pcf*>(shadow_map);

    glm::mat4 lvp = sm->get_projection() * sm->get_view();

    s->set(
        prefix + "map",
        shadow_sampler.bind(sm->depth, texture_index++)
    );
    s->set(prefix + "min_bias", sm->min_bias);
    s->set(prefix + "max_bias", sm->max_bias);
    s->set(prefix + "radius", sm->radius);
    s->set(prefix + "mvp", lvp * pos_to_world);
    s->set<int>(prefix + "samples", (int)sm->samples);
}

void shadow_pcf::set_shadow_map_uniforms(
    shader* s,
    unsigned& texture_index,
    omni_shadow_map* shadow_map,
    const std::string& prefix,
    const glm::mat4& pos_to_world
){
    omni_shadow_map_pcf* sm =
        static_cast<omni_shadow_map_pcf*>(shadow_map);

    s->set(
        prefix + "map",
        cubemap_shadow_sampler.bind(sm->depth, texture_index++)
    );
    s->set(prefix + "far_plane", sm->get_range().y);

    s->set(prefix + "min_bias", sm->min_bias);
    s->set(prefix + "max_bias", sm->max_bias);
    s->set(prefix + "radius", sm->radius);
    s->set<int>(prefix + "samples", (int)sm->samples);
}

void shadow_pcf::set_shadow_map_uniforms(
    shader* s,
    unsigned& texture_index,
    perspective_shadow_map* shadow_map,
    const std::string& prefix,
    const glm::mat4& pos_to_world
){
    perspective_shadow_map_pcf* sm =
        static_cast<perspective_shadow_map_pcf*>(shadow_map);

    glm::mat4 lvp = sm->get_projection() * sm->get_view();

    s->set(
        prefix + "map",
        shadow_sampler.bind(sm->depth, texture_index++)
    );
    s->set(prefix + "far_plane", sm->get_range().y);

    s->set(prefix + "min_bias", sm->min_bias);
    s->set(prefix + "max_bias", sm->max_bias);
    s->set(prefix + "radius", sm->radius);
    s->set(prefix + "mvp", lvp * pos_to_world);
    s->set<int>(prefix + "samples", (int)sm->samples);
}

void shadow_pcf::execute()
{
    if(!scene) return;


    const std::vector<directional_shadow_map*>* directional_shadow_maps = NULL;
    {
        const shadow_scene::directional_map& directional =
            scene->get_directional_shadows();
        auto it = directional.find(this);
        if(it != directional.end()) directional_shadow_maps = &it->second;
    }

    const std::vector<omni_shadow_map*>* omni_shadow_maps = NULL;
    {
        const shadow_scene::omni_map& omni = scene->get_omni_shadows();
        auto it = omni.find(this);
        if(it != omni.end()) omni_shadow_maps = &it->second;
    }

    const std::vector<perspective_shadow_map*>* perspective_shadow_maps = NULL;
    {
        const shadow_scene::perspective_map& perspective =
            scene->get_perspective_shadows();
        auto it = perspective.find(this);
        if(it != perspective.end()) perspective_shadow_maps = &it->second;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    if(directional_shadow_maps)
    {
        depth_shader->bind();

        // Directional shadow maps
        for(directional_shadow_map* sm: *directional_shadow_maps)
        {
            directional_shadow_map_pcf* pcf =
                static_cast<directional_shadow_map_pcf*>(sm);
            pcf->depth_buffer.bind();

            glClear(GL_DEPTH_BUFFER_BIT);

            glm::mat4 vp = pcf->get_projection() * pcf->get_view();

            for(object* obj: scene->get_objects())
            {
                const model* mod = obj->get_model();
                if(!mod) continue;

                glm::mat4 mvp = vp * obj->get_global_transform();
                depth_shader->set("mvp", mvp);

                for(const model::vertex_group& group: *mod)
                {
                    if(!group.mesh) continue;

                    group.mesh->draw();
                }
            }
        }
    }

    if(omni_shadow_maps)
    {
        cubemap_depth_shader->bind();

        // Omnidirectional shadow maps
        for(omni_shadow_map* sm: *omni_shadow_maps)
        {
            omni_shadow_map_pcf* pcf = static_cast<omni_shadow_map_pcf*>(sm);
            pcf->depth_buffer.bind();
            glClear(GL_DEPTH_BUFFER_BIT);

            glm::mat4 proj = pcf->get_projection();

            glm::mat4 face_vps[6] = {
                proj * pcf->get_view(0), proj * pcf->get_view(1),
                proj * pcf->get_view(2), proj * pcf->get_view(3),
                proj * pcf->get_view(4), proj * pcf->get_view(5)
            };
            cubemap_depth_shader->set("face_vps", 6, face_vps);
            cubemap_depth_shader->set(
                "pos", pcf->get_light()->get_global_position()
            );
            cubemap_depth_shader->set("far_plane", pcf->get_range().y);

            for(object* obj: scene->get_objects())
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

    if(perspective_shadow_maps)
    {
        perspective_depth_shader->bind();

        // Perspective shadow maps
        for(perspective_shadow_map* sm: *perspective_shadow_maps)
        {
            perspective_shadow_map_pcf* pcf =
                static_cast<perspective_shadow_map_pcf*>(sm);
            pcf->depth_buffer.bind();
            glClear(GL_DEPTH_BUFFER_BIT);

            glm::mat4 vp = pcf->get_projection() * pcf->get_view();

            perspective_depth_shader->set(
                "pos", pcf->get_light()->get_global_position()
            );
            perspective_depth_shader->set("far_plane", pcf->get_range().y);

            for(object* obj: scene->get_objects())
            {
                const model* mod = obj->get_model();
                if(!mod) continue;

                glm::mat4 m = obj->get_global_transform();
                glm::mat4 mvp = vp * m;
                perspective_depth_shader->set("m", m);
                perspective_depth_shader->set("mvp", mvp);

                for(const model::vertex_group& group: *mod)
                {
                    if(!group.mesh) continue;

                    group.mesh->draw();
                }
            }
        }
    }
}

std::string shadow_pcf::get_name() const
{
    return "shadow_pcf";
}

} // namespace lt::method

namespace lt
{

directional_shadow_map_pcf::directional_shadow_map_pcf(
    method::shadow_pcf* method,
    context& ctx,
    glm::uvec2 size,
    unsigned samples,
    float radius,
    glm::vec3 offset,
    glm::vec2 area,
    glm::vec2 depth_range,
    directional_light* light
):  directional_shadow_map(method, offset, area, depth_range, light),
    depth(
       ctx,
       size,
       GL_DEPTH_COMPONENT16,
       GL_FLOAT
    ),
    depth_buffer(ctx, size, {{GL_DEPTH_ATTACHMENT, {&depth}}}),
    radius(radius), samples(samples)
{
    set_bias();
}

directional_shadow_map_pcf::directional_shadow_map_pcf(
    directional_shadow_map_pcf&& other
):  directional_shadow_map(other),
    depth(std::move(other.depth)),
    depth_buffer(std::move(other.depth_buffer)),
    min_bias(other.min_bias), max_bias(other.max_bias),
    radius(other.radius), samples(other.samples)
{}

void directional_shadow_map_pcf::set_bias(float min_bias, float max_bias)
{
    this->min_bias = min_bias;
    this->max_bias = max_bias;
}

glm::vec2 directional_shadow_map_pcf::get_bias() const
{
    return glm::vec2(min_bias, max_bias);
}

void directional_shadow_map_pcf::set_samples(unsigned samples)
{
    this->samples = samples;
}

unsigned directional_shadow_map_pcf::get_samples() const
{
    return samples;
}

void directional_shadow_map_pcf::set_radius(float radius)
{
    this->radius = radius;
}

float directional_shadow_map_pcf::set_radius() const
{
    return radius;
}

texture& directional_shadow_map_pcf::get_depth()
{
    return depth;
}

const texture& directional_shadow_map_pcf::get_depth() const
{
    return depth;
}

omni_shadow_map_pcf::omni_shadow_map_pcf(
    method::shadow_pcf* method,
    context& ctx,
    glm::uvec2 size,
    unsigned samples,
    float radius,
    glm::vec2 depth_range,
    point_light* light
):  omni_shadow_map(method, depth_range, light),
    depth(
       ctx,
       size,
       GL_DEPTH_COMPONENT16,
       GL_FLOAT,
       0,
       GL_TEXTURE_CUBE_MAP
    ),
    depth_buffer(
        ctx,
        size,
        {{GL_DEPTH_ATTACHMENT, {&depth}}},
        GL_TEXTURE_CUBE_MAP
    ),
    radius(radius), samples(samples)
{
    set_bias();
}

omni_shadow_map_pcf::omni_shadow_map_pcf(
    omni_shadow_map_pcf&& other
):  omni_shadow_map(other),
    depth(std::move(other.depth)),
    depth_buffer(std::move(other.depth_buffer)),
    min_bias(other.min_bias), max_bias(other.max_bias),
    radius(other.radius), samples(other.samples)
{}

void omni_shadow_map_pcf::set_bias(float min_bias, float max_bias)
{
    this->min_bias = min_bias;
    this->max_bias = max_bias;
}

glm::vec2 omni_shadow_map_pcf::get_bias() const
{
    return glm::vec2(min_bias, max_bias);
}

void omni_shadow_map_pcf::set_samples(unsigned samples)
{
    this->samples = samples;
}

unsigned omni_shadow_map_pcf::get_samples() const
{
    return samples;
}

void omni_shadow_map_pcf::set_radius(float radius)
{
    this->radius = radius;
}

float omni_shadow_map_pcf::set_radius() const
{
    return radius;
}

texture& omni_shadow_map_pcf::get_depth()
{
    return depth;
}

const texture& omni_shadow_map_pcf::get_depth() const
{
    return depth;
}

perspective_shadow_map_pcf::perspective_shadow_map_pcf(
    method::shadow_pcf* method,
    context& ctx,
    glm::uvec2 size,
    unsigned samples,
    float radius,
    double fov,
    glm::vec2 depth_range,
    point_light* light
):  perspective_shadow_map(method, fov, depth_range, light),
    depth(
       ctx,
       size,
       GL_DEPTH_COMPONENT16,
       GL_FLOAT
    ),
    depth_buffer(ctx, size, {{GL_DEPTH_ATTACHMENT, {&depth}}}),
    radius(radius), samples(samples)
{
    set_bias();
}

perspective_shadow_map_pcf::perspective_shadow_map_pcf(
    perspective_shadow_map_pcf&& other
):  perspective_shadow_map(other),
    depth(std::move(other.depth)),
    depth_buffer(std::move(other.depth_buffer)),
    min_bias(other.min_bias), max_bias(other.max_bias),
    radius(other.radius), samples(other.samples)
{}

void perspective_shadow_map_pcf::set_bias(float min_bias, float max_bias)
{
    this->min_bias = min_bias;
    this->max_bias = max_bias;
}

glm::vec2 perspective_shadow_map_pcf::get_bias() const
{
    return glm::vec2(min_bias, max_bias);
}

void perspective_shadow_map_pcf::set_samples(unsigned samples)
{
    this->samples = samples;
}

unsigned perspective_shadow_map_pcf::get_samples() const
{
    return samples;
}

void perspective_shadow_map_pcf::set_radius(float radius)
{
    this->radius = radius;
}

float perspective_shadow_map_pcf::set_radius() const
{
    return radius;
}

texture& perspective_shadow_map_pcf::get_depth()
{
    return depth;
}

const texture& perspective_shadow_map_pcf::get_depth() const
{
    return depth;
}

} // namespace lt
