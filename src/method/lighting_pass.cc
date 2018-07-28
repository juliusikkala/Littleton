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
#include "lighting_pass.hh"
#include "multishader.hh"
#include "camera.hh"
#include "math.hh"
#include "gbuffer.hh"
#include "resource_pool.hh"
#include "scene.hh"
#include "common_resources.hh"
#include "shadow_map.hh"
#include "shadow_method.hh"

namespace 
{
using namespace lt;
using namespace lt::method;

void set_gbuf(shader* s, gbuffer* buf, const camera* cam)
{
    unsigned start_index = 0;
    buf->set_uniforms(s, start_index);
    s->set("projection_info", cam->get_projection_info());
    s->set("clip_info", cam->get_clip_info());
}

float compute_cutoff_radius(light* light, float cutoff)
{
    glm::vec3 radius2 = light->get_color()/cutoff;
    return sqrt(glm::max(glm::max(radius2.x, radius2.y), radius2.z));
}

// Returns false if the light shouldn't be rendered at all.
bool set_bounding_rect(
    shader* s, point_light* light, glm::vec3 light_pos,
    const camera* cam, float cutoff,
    lighting_pass::depth_test light_test
){
    glm::mat4 m = glm::mat4(1.0f);

    if(cutoff > 0)
    {
        float r = compute_cutoff_radius(light, cutoff);

        // No need to render the light if it isn't in the frustum anyway.
        if(!cam->sphere_is_visible(light_pos, r)) return false;

        m = cam->get_projection() * sphere_projection_quad_matrix(
            light_pos,
            r,
            cam->get_near(),
            cam->get_far(),
            light_test == lighting_pass::TEST_NEAR
        );
    }

    s->set("m", m);
    s->set("mvp", m);
    return true;
}

bool set_light(
    shader* s,
    point_light* light,
    const camera* cam,
    float cutoff,
    lighting_pass::depth_test light_test
){
    glm::mat4 inv_view = cam->get_global_transform();
    glm::vec3 pos = glm::vec3(
        glm::inverse(inv_view) * glm::vec4(light->get_global_position(), 1)
    );
    s->set("inv_view", inv_view);
    s->set("light.position", pos);
    s->set("light.color", light->get_color());
    return set_bounding_rect(s, light, pos, cam, cutoff, light_test);
}

bool set_light(
    shader* s,
    spotlight* light,
    const camera* cam,
    float cutoff,
    lighting_pass::depth_test light_test
){
    glm::mat4 inv_view = cam->get_global_transform();
    glm::mat4 view = glm::inverse(inv_view);
    glm::vec3 pos = glm::vec3(
        view * glm::vec4(light->get_global_position(), 1)
    );

    s->set("inv_view", inv_view);
    s->set("light.position", pos);
    s->set("light.color", light->get_color());

    s->set(
        "light.direction",
        glm::normalize(glm::vec3(
            view * glm::vec4(light->get_global_direction(), 0)
        ))
    );
    
    s->set<float>(
        "light.cutoff",
        cos(glm::radians(light->get_cutoff_angle()))
    );
    s->set(
        "light.exponent",
        light->get_falloff_exponent()
    );

    return set_bounding_rect(s, light, pos, cam, cutoff, light_test);
}

bool set_light(
    shader* s,
    directional_light* light,
    const camera* cam
){
    glm::mat4 inv_view = cam->get_global_transform();
    s->set("light.color", light->get_color());
    s->set(
        "light.direction",
        glm::normalize(glm::vec3(
            glm::inverse(inv_view) * glm::vec4(light->get_direction(), 0)
        ))
    );

    glm::mat4 m = glm::mat4(1.0f);
    s->set("m", m);
    s->set("mvp", m);

    return true;
}

template<typename L>
void render_shadowed(
    gbuffer* buf,
    multishader* lighting_shader,
    const shadow_scene::omni_map& shadows,
    const shader::definition_map& light_definitions,
    const std::vector<L*>& lights,
    std::vector<bool>& handled_lights,
    const camera* cam,
    float cutoff,
    lighting_pass::depth_test light_test,
    const primitive& quad,
    unsigned start_index
){
    for(const auto& pair: shadows)
    {
        shadow_method* m = pair.first;
        shader::definition_map def(m->get_omni_definitions());

        def.insert(light_definitions.begin(), light_definitions.end());

        shader* s = lighting_shader->get(def);
        s->bind();

        unsigned texture_index = start_index;
        m->set_omni_uniforms(s, texture_index);

        for(omni_shadow_map* sm: pair.second)
        {
            unsigned local_texture_index = texture_index;
            L* light = static_cast<L*>(sm->get_light());

            // Check if the light is in the scene and mark it as handled
            auto it = std::lower_bound(lights.begin(), lights.end(), light);
            if(it == lights.end() || *it != light) continue;
            handled_lights[it - lights.begin()] = true;

            if(!set_light(s, light, cam, cutoff, light_test)) continue;

            m->set_shadow_map_uniforms(
                s, local_texture_index, sm,
                "shadow.", cam->get_global_transform()
            );

            set_gbuf(s, buf, cam);

            quad.draw();
        }
    }
}

template<typename L>
void render_shadowed(
    gbuffer* buf,
    multishader* lighting_shader,
    const shadow_scene::perspective_map& shadows,
    const shader::definition_map& light_definitions,
    const std::vector<L*>& lights,
    std::vector<bool>& handled_lights,
    const camera* cam,
    float cutoff,
    lighting_pass::depth_test light_test,
    const primitive& quad,
    unsigned start_index
){
    for(const auto& pair: shadows)
    {
        shadow_method* m = pair.first;
        shader::definition_map def(m->get_perspective_definitions());

        def.insert(light_definitions.begin(), light_definitions.end());

        shader* s = lighting_shader->get(def);
        s->bind();

        unsigned texture_index = start_index;
        m->set_omni_uniforms(s, texture_index);

        for(perspective_shadow_map* sm: pair.second)
        {
            unsigned local_texture_index = texture_index;
            L* light = static_cast<L*>(sm->get_light());

            // Check if the light is in the scene and mark it as handled
            auto it = std::lower_bound(lights.begin(), lights.end(), light);
            if(it == lights.end() || *it != light) continue;
            handled_lights[it - lights.begin()] = true;

            if(!set_light(s, light, cam, cutoff, light_test)) continue;

            m->set_shadow_map_uniforms(
                s, local_texture_index, sm,
                "shadow.", cam->get_global_transform()
            );

            set_gbuf(s, buf, cam);

            quad.draw();
        }
    }
}

void render_point_lights(
    gbuffer* buf,
    multishader* lighting_shader,
    camera_scene* cameras,
    light_scene* lights,
    shadow_scene* shadows,
    float cutoff,
    lighting_pass::depth_test light_test,
    const primitive& quad,
    bool visualize_light_volumes,
    unsigned start_index
){
    const std::vector<point_light*>& l = lights->get_point_lights();
    std::vector<bool> handled_lights(l.size(), false);

    shader::definition_map definitions({{"POINT_LIGHT", ""}});
    if(visualize_light_volumes) definitions["VISUALIZE"];
    quad.update_definitions(definitions);

    camera* cam = cameras->get_camera();

    // Render shadowed lights
    render_shadowed(
        buf,
        lighting_shader,
        shadows->get_omni_shadows(),
        definitions,
        l,
        handled_lights,
        cam,
        cutoff,
        light_test,
        quad,
        start_index
    );

    render_shadowed(
        buf,
        lighting_shader,
        shadows->get_perspective_shadows(),
        definitions,
        l,
        handled_lights,
        cam,
        cutoff,
        light_test,
        quad,
        start_index
    );

    // Render unshadowed lights
    shader* s = lighting_shader->get(definitions);
    s->bind();
    set_gbuf(s, buf, cam);

    for(unsigned i = 0; i < l.size(); ++i)
    {
        if(handled_lights[i]) continue;
        if(!set_light(s, l[i], cam, cutoff, light_test))
            continue;
        quad.draw();
    }
}

void render_spotlights(
    gbuffer* buf,
    multishader* lighting_shader,
    camera_scene* cameras,
    light_scene* lights,
    shadow_scene* shadows,
    float cutoff,
    lighting_pass::depth_test light_test,
    const primitive& quad,
    bool visualize_light_volumes,
    unsigned start_index
){
    const std::vector<spotlight*>& l = lights->get_spotlights();
    std::vector<bool> handled_lights(l.size(), false);

    shader::definition_map definitions({{"SPOTLIGHT", ""}});
    if(visualize_light_volumes) definitions["VISUALIZE"];
    quad.update_definitions(definitions);

    camera* cam = cameras->get_camera();

    // Render shadowed lights
    render_shadowed(
        buf,
        lighting_shader,
        shadows->get_omni_shadows(),
        definitions,
        l,
        handled_lights,
        cam,
        cutoff,
        light_test,
        quad,
        start_index
    );

    render_shadowed(
        buf,
        lighting_shader,
        shadows->get_perspective_shadows(),
        definitions,
        l,
        handled_lights,
        cam,
        cutoff,
        light_test,
        quad,
        start_index
    );

    // Render unshadowed lights
    shader* s = lighting_shader->get(definitions);
    s->bind();
    set_gbuf(s, buf, cam);

    for(unsigned i = 0; i < l.size(); ++i)
    {
        if(handled_lights[i]) continue;
        if(!set_light(s, l[i], cam, cutoff, light_test))
            continue;
        quad.draw();
    }
}

void render_directional_lights(
    gbuffer* buf,
    multishader* lighting_shader,
    camera_scene* cameras,
    light_scene* lights,
    shadow_scene* shadows,
    const primitive& quad,
    unsigned start_index
){
    const std::vector<directional_light*>& l = lights->get_directional_lights();
    std::vector<bool> handled_lights(l.size(), false); 
    shader::definition_map definitions({{"DIRECTIONAL_LIGHT", ""}});
    quad.update_definitions(definitions);

    camera* cam = cameras->get_camera();

    // Render shadowed lights
    for(const auto& pair: shadows->get_directional_shadows())
    {
        shadow_method* m = pair.first;
        shader::definition_map def(m->get_directional_definitions());

        def.insert(definitions.begin(), definitions.end());

        shader* s = lighting_shader->get(def);
        s->bind();

        unsigned texture_index = start_index;
        m->set_directional_uniforms(s, texture_index);

        for(directional_shadow_map* sm: pair.second)
        {
            unsigned local_texture_index = texture_index;
            directional_light* light = sm->get_light();

            // Check if the light is in the scene and mark it as handled
            auto it = std::lower_bound(l.begin(), l.end(), light);
            if(it == l.end() || *it != light) continue;
            handled_lights[it - l.begin()] = true;

            if(!set_light(s, light, cam)) continue;

            m->set_shadow_map_uniforms(
                s, local_texture_index, sm,
                "shadow.", cam->get_global_transform()
            );

            set_gbuf(s, buf, cam);

            quad.draw();
        }
    }

    // Render unshadowed lights
    shader* s = lighting_shader->get(definitions);
    s->bind();
    set_gbuf(s, buf, cam);

    for(unsigned i = 0; i < l.size(); ++i)
    {
        if(handled_lights[i]) continue;
        if(!set_light(s, l[i], cam)) continue;
        quad.draw();
    }
}

}

namespace lt::method
{

lighting_pass::lighting_pass(
    render_target& target,
    gbuffer& buf,
    resource_pool& pool,
    Scene scene,
    float cutoff
):  target_method(target), scene_method(scene), buf(&buf),
    lighting_shader(pool.get_shader(
        shader::path{"generic.vert", "lighting.frag"}
    )),
    cutoff(cutoff), light_test(TEST_NEAR), visualize_light_volumes(false),
    quad(common::ensure_quad_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool))
{
}

void lighting_pass::set_cutoff(float cutoff)
{
    this->cutoff = cutoff;
}

float lighting_pass::get_cutoff() const
{
    return cutoff;
}

void lighting_pass::set_light_depth_test(depth_test test)
{
    this->light_test = test;
}

lighting_pass::depth_test lighting_pass::get_light_depth_test() const
{
    return light_test;
}

void lighting_pass::set_visualize_light_volumes(bool visualize)
{
    visualize_light_volumes = visualize;
}

void lighting_pass::execute()
{
    target_method::execute();

    if(!lighting_shader || !has_all_scenes())
        return;

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    stencil_cull();

    if(cutoff > 0 && light_test != TEST_NONE)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        if(light_test == TEST_FAR) glDepthFunc(GL_GEQUAL);
        else glDepthFunc(GL_LEQUAL);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    camera* cam = get_scene<camera_scene>()->get_camera();
    if(!cam) return;

    unsigned texture_index = 0;
    buf->bind_textures(fb_sampler, texture_index);

    render_point_lights(
        buf, lighting_shader,
        get_scene<camera_scene>(),
        get_scene<light_scene>(),
        get_scene<shadow_scene>(),
        cutoff, light_test,
        quad, visualize_light_volumes,
        texture_index
    );

    render_spotlights(
        buf, lighting_shader,
        get_scene<camera_scene>(),
        get_scene<light_scene>(),
        get_scene<shadow_scene>(),
        cutoff, light_test,
        quad, visualize_light_volumes,
        texture_index
    );

    if(cutoff > 0 && light_test != TEST_NONE)
    {
        glDisable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_TRUE);
    }

    render_directional_lights(
        buf,
        lighting_shader,
        get_scene<camera_scene>(),
        get_scene<light_scene>(),
        get_scene<shadow_scene>(),
        quad,
        texture_index
    );
}

std::string lighting_pass::get_name() const
{
    return "lighting_pass";
}

} // namespace lt::method
