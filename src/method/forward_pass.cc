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
#include "forward_pass.hh"
#include "context.hh"
#include "camera.hh"
#include "model.hh"
#include "object.hh"
#include "material.hh"
#include "math.hh"
#include "multishader.hh"
#include "resource_pool.hh"
#include "scene.hh"
#include "primitive.hh"
#include "shadow_map.hh"
#include "gbuffer.hh"
#include "shadow_method.hh"
#include "common_resources.hh"

namespace
{
using namespace lt;
using namespace lt::method;

void set_light(
    shader* s,
    point_light* light,
    const glm::mat4& view
){
    s->set(
        "light.position",
        glm::vec3(view * glm::vec4(light->get_global_position(), 1))
    );
    s->set("light.color", light->get_color());
}

void set_light(
    shader* s,
    spotlight* light,
    const glm::mat4& view
){
    s->set(
        "light.position",
        glm::vec3(view * glm::vec4(light->get_global_position(), 1))
    );
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
    s->set("light.exponent", light->get_falloff_exponent());
}

void set_light(
    shader* s,
    directional_light* light,
    const glm::mat4& view
){
    s->set("light.color", light->get_color());
    s->set(
        "light.direction",
        glm::normalize(glm::vec3(
            view * glm::vec4(light->get_direction(), 0)
        ))
    );
}

void set_shadow(
    shadow_method* met,
    shader* s,
    unsigned& texture_index,
    directional_shadow_map* sm,
    const glm::mat4& m
){
    met->set_directional_uniforms(s, texture_index);
    met->set_shadow_map_uniforms(s, texture_index, sm, "shadow.", m);
}

void set_shadow(
    shadow_method* met,
    shader* s,
    unsigned& texture_index,
    omni_shadow_map* sm,
    const glm::mat4& m
){
    met->set_omni_uniforms(s, texture_index);
    met->set_shadow_map_uniforms(s, texture_index, sm, "shadow.", m);
}

void set_shadow(
    shadow_method* met,
    shader* s,
    unsigned& texture_index,
    perspective_shadow_map* sm,
    const glm::mat4& m
){
    met->set_perspective_uniforms(s, texture_index);
    met->set_shadow_map_uniforms(s, texture_index, sm, "shadow.", m);
}

template<typename F>
void render_pass(
    render_target& target,
    multishader* forward_shader,
    bool world_space,
    render_scene* scene,
    const shader::definition_map& common,
    bool potentially_transparent_only,
    F&& vertex_group_callback
){
    bool cubemap_target =
        target.get_target() == GL_TEXTURE_CUBE_MAP ||
        target.get_target() == GL_TEXTURE_CUBE_MAP_ARRAY;

    unsigned layers = min(
        (unsigned)scene->camera_count(), target.get_dimensions().z
    );

    camera* cam = scene->get_camera();

    // Generate cubemap face-layer view-projection matrices
    const std::vector<camera*> cameras = scene->get_cameras();
    std::vector<glm::mat4> face_layer_vps;
    if(cubemap_target)
    {
        face_layer_vps.reserve(layers*6);
        for(unsigned layer = 0; layer < layers; ++layer)
            for(unsigned face = 0; face < 6; ++face)
                face_layer_vps.push_back(
                    cameras[layer]->get_view_projection(face)
                );
    }

    glm::mat4 inv_view = cam->get_global_transform();
    glm::mat4 v = glm::inverse(inv_view);
    glm::mat4 p = cam->get_projection();

    // Loop objects in scene
    for(object* obj: scene->get_objects())
    {
        const model* mod = obj->get_model();
        if(!mod) continue;

        glm::mat4 m = obj->get_global_transform();
        glm::mat4 mv = v * m;
        glm::mat3 n_m(glm::inverseTranspose(world_space ? m : mv));
        glm::mat4 mvp = p * mv;

        // Loop vertex groups in the object's model
        for(const model::vertex_group& group: *mod)
        {
            if(!group.mat || !group.mesh) continue;
            // Skip certainly opaque objects if only transparent stuff should be
            // rendered.
            if(!group.mat->potentially_transparent() &&
               potentially_transparent_only) continue;

            shader::definition_map def(common);
            group.mat->update_definitions(def);
            group.mesh->update_definitions(def);

            shader* s = forward_shader->get(def);
            s->bind();

            unsigned texture_index = 0;
            group.mat->apply(s, texture_index);

            vertex_group_callback(s, texture_index, m, v);

            if(cubemap_target)
            {
                s->set(
                    "face_vps",
                    face_layer_vps.size(),
                    face_layer_vps.data()
                );
                s->set("mvp", m);
            }
            else s->set("mvp", mvp);

            s->set("m", world_space ? m : mv);
            s->set("n_m", n_m);
            s->set("inv_view", inv_view);
            s->set("camera_pos", cam->get_position());

            group.mesh->draw();
        }
    }
}

template<typename L, typename S>
void render_shadowed_light(
    render_target& target,
    shadow_method* met,
    const shader::definition_map& scene_definitions,
    render_scene* scene,
    multishader* forward_shader,
    bool world_space,
    L* light,
    S* sm,
    bool potentially_transparent_only
){
    render_pass(
        target, forward_shader, world_space, scene, scene_definitions,
        potentially_transparent_only,
        [&](
            shader* s,
            unsigned& texture_index,
            const glm::mat4& m,
            const glm::mat4& v
        ){
            set_shadow(met, s, texture_index, sm, m);
            set_light(s, light, world_space ? glm::mat4(1) : v);
        }
    );
}

void render_shadowed_lights(
    render_target& target,
    multishader* forward_shader,
    bool world_space,
    std::vector<bool>& handled_point_lights,
    std::vector<bool>& handled_spotlights,
    std::vector<bool>& handled_directional_lights,
    render_scene* scene,
    const shader::definition_map& common,
    bool potentially_transparent_only
){
    // Directional shadows are a bit simpler to use since they are always bound
    // to only one light type, directional_light.
    shader::definition_map directional_def(common);
    directional_def["DIRECTIONAL_LIGHT"];
    directional_def["SINGLE_LIGHT"];
    directional_def["OUTPUT_LIGHTING"];

    const std::vector<directional_light*>& directional_lights =
        scene->get_directional_lights();

    for(const auto& pair: scene->get_directional_shadows())
    {
        shadow_method* met = pair.first;

        shader::definition_map scene_definitions(
            met->get_directional_definitions()
        );

        scene_definitions.insert(
            directional_def.begin(),
            directional_def.end()
        );

        for(directional_shadow_map* sm: pair.second)
        {
            directional_light* light = sm->get_light();

            auto it = std::lower_bound(
                directional_lights.begin(),
                directional_lights.end(),
                light
            );
            if(it == directional_lights.end() || *it != light) continue;
            handled_directional_lights[it - directional_lights.begin()] = true;

            render_shadowed_light(
                target, met, scene_definitions, scene, forward_shader,
                world_space, light, sm, potentially_transparent_only
            );
        }
    }

    shader::definition_map point_def(common);
    point_def["POINT_LIGHT"];
    point_def["SINGLE_LIGHT"];
    point_def["OUTPUT_LIGHTING"];

    shader::definition_map spot_def(common);
    spot_def["SPOTLIGHT"];
    spot_def["SINGLE_LIGHT"];
    spot_def["OUTPUT_LIGHTING"];

    const std::vector<point_light*>& point_lights = scene->get_point_lights();
    const std::vector<spotlight*>& spotlights = scene->get_spotlights();

    for(const auto& pair: scene->get_omni_shadows())
    {
        shadow_method* met = pair.first;

        shader::definition_map scene_definitions(met->get_omni_definitions());
        shader::definition_map point_definitions(scene_definitions);
        point_definitions.insert(point_def.begin(), point_def.end());
        shader::definition_map spot_definitions(scene_definitions);
        spot_definitions.insert(spot_def.begin(), spot_def.end());

        for(omni_shadow_map* sm: pair.second)
        {
            point_light* point = sm->get_light();
            spotlight* spot = static_cast<spotlight*>(point);

            auto point_it = std::lower_bound(
                point_lights.begin(),
                point_lights.end(),
                point
            );

            auto spot_it = std::lower_bound(
                spotlights.begin(),
                spotlights.end(),
                spot
            );

            if(point_it != point_lights.end() && *point_it == point)
            {// Handle point light
                handled_point_lights[point_it - point_lights.begin()] = true;

                render_shadowed_light(
                    target, met, point_definitions, scene, forward_shader,
                    world_space, point, sm, potentially_transparent_only
                );
            }
            else if(spot_it != spotlights.end() && *spot_it == spot)
            {// Handle spotlight
                handled_spotlights[spot_it - spotlights.begin()] = true;

                render_shadowed_light(
                    target, met, spot_definitions, scene, forward_shader,
                    world_space, spot, sm, potentially_transparent_only
                );
            }
        }
    }

    for(const auto& pair: scene->get_perspective_shadows())
    {
        shadow_method* met = pair.first;

        shader::definition_map scene_definitions(
            met->get_perspective_definitions()
        );
        shader::definition_map point_definitions(scene_definitions);
        point_definitions.insert(point_def.begin(), point_def.end());
        shader::definition_map spot_definitions(scene_definitions);
        spot_definitions.insert(spot_def.begin(), spot_def.end());

        for(perspective_shadow_map* sm: pair.second)
        {
            point_light* point = sm->get_light();
            spotlight* spot = static_cast<spotlight*>(point);

            auto point_it = std::lower_bound(
                point_lights.begin(),
                point_lights.end(),
                point
            );

            auto spot_it = std::lower_bound(
                spotlights.begin(),
                spotlights.end(),
                spot
            );

            if(point_it != point_lights.end() && *point_it == point)
            {
                handled_point_lights[point_it - point_lights.begin()] = true;

                render_shadowed_light(
                    target, met, point_definitions, scene, forward_shader,
                    world_space, point, sm, potentially_transparent_only
                );
            }
            else if(spot_it != spotlights.end() && *spot_it == spot)
            {
                handled_spotlights[spot_it - spotlights.begin()] = true;

                render_shadowed_light(
                    target, met, spot_definitions, scene, forward_shader,
                    world_space, spot, sm, potentially_transparent_only
                );
            }
        }
    }
}

std::unique_ptr<uniform_block> create_light_block(
    const std::string& block_name,
    render_scene* scene,
    shader* compatible_shader,
    const std::vector<bool>& handled_point_lights,
    const std::vector<bool>& handled_spotlights,
    const std::vector<bool>& handled_directional_lights,
    const glm::mat4& view
){
    unsigned point_light_count = 0;
    unsigned spotlight_count = 0;
    unsigned directional_light_count = 0;

    std::unique_ptr<uniform_block> light_block(
        new uniform_block(compatible_shader->get_block_type(block_name))
    );

    const std::vector<point_light*>& point_lights = scene->get_point_lights();
    for(unsigned i = 0; i < point_lights.size(); ++i)
    {
        if(handled_point_lights[i]) continue;
        point_light* l = point_lights[i];

        std::string prefix = "point["+std::to_string(point_light_count)+"].";
        point_light_count++;

        light_block->set(prefix + "color", l->get_color());
        light_block->set(
            prefix + "position",
            glm::vec3(view * glm::vec4(l->get_global_position(), 1))
        );
    }

    const std::vector<spotlight*>& spotlights = scene->get_spotlights();
    for(unsigned i = 0; i < spotlights.size(); ++i)
    {
        if(handled_spotlights[i]) continue;
        spotlight* l = spotlights[i];

        std::string prefix = "spot["+std::to_string(spotlight_count)+"].";
        spotlight_count++;

        light_block->set(prefix + "color", l->get_color());
        light_block->set(
            prefix + "position",
            glm::vec3(view * glm::vec4(l->get_global_position(), 1))
        );
        light_block->set(
            prefix + "direction",
            glm::normalize(glm::vec3(
                view * glm::vec4(l->get_global_direction(), 0)
            ))
        );
        light_block->set<float>(
            prefix + "cutoff",
            cos(glm::radians(l->get_cutoff_angle()))
        );
        light_block->set(
            prefix + "exponent",
            l->get_falloff_exponent()
        );
    }

    const std::vector<directional_light*>& directional_lights =
        scene->get_directional_lights();

    for(unsigned i = 0; i < directional_lights.size(); ++i)
    {
        if(handled_directional_lights[i]) continue;
        directional_light* l = directional_lights[i];

        std::string prefix =
            "directional["+std::to_string(directional_light_count)+"].";
        directional_light_count++;

        light_block->set(
            prefix + "color",
            l->get_color()
        );
        light_block->set(
            prefix + "direction",
            glm::normalize(glm::vec3(
                view * glm::vec4(l->get_direction(), 0)
            ))
        );
    }

    light_block->set<int>("point_light_count", point_light_count);
    light_block->set<int>("directional_light_count", directional_light_count);
    light_block->set<int>("spotlight_count", spotlight_count);

    light_block->upload();

    return light_block;
}


void update_scene_definitions(
    shader::definition_map& def,
    render_scene* scene
){
    def["MULTIPLE_LIGHTS"];
    def["MAX_POINT_LIGHT_COUNT"] = std::to_string(
        next_power_of_two(scene->point_light_count()));
    def["MAX_DIRECTIONAL_LIGHT_COUNT"] = std::to_string(
        next_power_of_two(scene->directional_light_count()));
    def["MAX_SPOTLIGHT_COUNT"] = std::to_string(
        next_power_of_two(scene->spotlight_count()));
}

void render_unshadowed_lights(
    render_target& target,
    multishader* forward_shader,
    bool world_space,
    const std::vector<bool>& handled_point_lights,
    const std::vector<bool>& handled_spotlights,
    const std::vector<bool>& handled_directional_lights,
    render_scene* scene,
    const shader::definition_map& common,
    bool potentially_transparent_only
){
    shader::definition_map scene_definitions(common);
    update_scene_definitions(scene_definitions, scene);

    std::unique_ptr<uniform_block> light_block;

    render_pass(
        target,
        forward_shader,
        world_space,
        scene,
        scene_definitions,
        potentially_transparent_only,
        [&](
            shader* s,
            unsigned& texture_index,
            const glm::mat4& m,
            const glm::mat4& v
        ){
            // Generate the light block when the first shader containing it
            // exists (the structure of the light block can't be known
            // beforehand)
            if(!light_block && s->block_exists("Lights"))
            {
                light_block = create_light_block(
                    "Lights",
                    scene,
                    s,
                    handled_point_lights,
                    handled_spotlights,
                    handled_directional_lights,
                    world_space ? glm::mat4(1) : v
                );
                light_block->bind(0);
            }
            if(light_block) s->set_block("Lights", 0);
            s->set("ambient", scene->get_ambient());
        }
    );
}

void depth_pass(
    render_target& target,
    multishader* depth_shader,
    bool world_space,
    render_scene* scene,
    const shader::definition_map& common,
    bool potentially_transparent_only
){
    render_pass(
        target, depth_shader, world_space, scene, common,
        potentially_transparent_only,
        [&](
            shader* s,
            unsigned& texture_index,
            const glm::mat4& m,
            const glm::mat4& v
        ){
            s->set("ambient", scene->get_ambient());
        }
    );
}

void render_forward_pass(
    render_target& target,
    render_scene* scene,
    bool world_space,
    bool opaque,
    bool apply_ambient,
    bool transmittance,
    stencil_handler& stencil,
    gbuffer* gbuf,
    multishader* forward_shader
){
    camera* cam = scene->get_camera();
    if(!cam) return;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    std::vector<bool> handled_point_lights(scene->point_light_count(), false);
    std::vector<bool> handled_spotlights(scene->spotlight_count(), false);
    std::vector<bool> handled_directional_lights(
        scene->directional_light_count(), false
    );

    shader::definition_map common_def({{"OUTPUT_LIGHTING", ""}});
    if(world_space) common_def["WORLD_SPACE"];
    if(opaque) common_def["MIN_ALPHA"] = "1.0f";
    else
    {
        common_def["MAX_ALPHA"] = "1.0f";
        common_def["MIN_ALPHA"] = std::to_string(1/255.0f);
    }

    unsigned layers = min(
        (unsigned)scene->camera_count(), target.get_dimensions().z
    );
    common_def["LAYERS"] = std::to_string(layers);
    common_def["CUBEMAP_MAX_VERTICES"] = std::to_string(3*6*layers);

    shader::definition_map depth_def(common_def);

    if(gbuf)
    {
        depth_def["OUTPUT_GEOMETRY"];
        gbuf->set_draw(gbuffer::DRAW_ALL);
        gbuf->update_definitions(depth_def);
    }

    glDisable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);

    if(gbuf)
    {
        if(!opaque)
        {
            glColorMaski(
                gbuf->get_lighting_index(),
                GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE
            );
        }

        stencil.stencil_draw();
        depth_pass(
            target,
            forward_shader,
            world_space,
            scene,
            depth_def,
            !opaque
        );
        stencil.stencil_disable();

        if(!opaque)
        {
            glColorMaski(
                gbuf->get_lighting_index(),
                GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE
            );
        }

        gbuf->set_draw(gbuffer::DRAW_LIGHTING);
        if(!opaque && transmittance)
        {
            glEnable(GL_BLEND);
            glBlendFunci(
                gbuf->get_lighting_index(),
                GL_ZERO,
                GL_ONE_MINUS_SRC_ALPHA
            );
            depth_def.erase("OUTPUT_GEOMETRY");
            depth_pass(
                target,
                forward_shader,
                world_space,
                scene,
                depth_def,
                !opaque
            );
        }
    }
    else
    {
        if(!opaque) glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        stencil.stencil_draw();
        depth_pass(
            target,
            forward_shader,
            world_space,
            scene,
            depth_def,
            !opaque
        );
        stencil.stencil_disable();

        if(!opaque) glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        if(!opaque && transmittance)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
            depth_pass(
                target,
                forward_shader, 
                world_space,
                scene,
                depth_def,
                !opaque
            );
        }
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    render_shadowed_lights(
        target,
        forward_shader,
        world_space,
        handled_point_lights,
        handled_spotlights,
        handled_directional_lights,
        scene,
        common_def,
        !opaque
    );

    if(apply_ambient) common_def["APPLY_AMBIENT"];
    common_def["APPLY_EMISSION"];

    render_unshadowed_lights(
        target,
        forward_shader,
        world_space,
        handled_point_lights,
        handled_spotlights,
        handled_directional_lights,
        scene,
        common_def,
        !opaque
    );
}

}

namespace lt::method
{
forward_pass::forward_pass(
    render_target& target,
    resource_pool& pool,
    render_scene* scene,
    bool apply_ambient,
    bool apply_transmittance
):  target_method(target),
    forward_shader(pool.get_shader(
        shader::path{"generic.vert", "forward.frag"})
    ),
    cubemap_forward_shader(pool.get_shader(
        shader::path{"generic.vert", "forward.frag", "cubemap.geom"})
    ),
    min_max_shader(nullptr),
    scene(scene), gbuf(nullptr),
    opaque(true), transparent(true), apply_ambient(apply_ambient),
    apply_transmittance(apply_transmittance),
    quad(common::ensure_quad_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool))
{}

forward_pass::forward_pass(
    gbuffer& buf,
    resource_pool& pool,
    render_scene* scene,
    bool apply_ambient,
    bool apply_transmittance
):  forward_pass(
        (render_target&)buf, pool, scene, apply_ambient, apply_transmittance
    )
{
    min_max_shader = buf.get_min_max_shader(pool);
    gbuf = &buf;
}

forward_pass::~forward_pass() {}

void forward_pass::execute()
{
    target_method::execute();
    if(!forward_shader || !scene)
        return;

    bool cubemap =
        get_target().get_target() == GL_TEXTURE_CUBE_MAP ||
        get_target().get_target() == GL_TEXTURE_CUBE_MAP_ARRAY;

    if(opaque)
    {
        render_forward_pass(
            get_target(),
            scene,
            cubemap,
            true,
            apply_ambient,
            apply_transmittance,
            *this,
            gbuf,
            cubemap ? cubemap_forward_shader : forward_shader
        );
    }

    if(transparent)
    {
        render_forward_pass(
            get_target(),
            scene,
            cubemap,
            false,
            apply_ambient,
            apply_transmittance,
            *this,
            gbuf,
            cubemap ? cubemap_forward_shader : forward_shader
        );
    }

    // TODO: Separate method for gbuffer mipmap rendering, this one might render
    // unnecessarily in hybrid pipelines.
    if(gbuf) gbuf->render_depth_mipmaps(min_max_shader, quad, fb_sampler);
}

void forward_pass::set_scene(render_scene* s) { scene = s; }

render_scene* forward_pass::get_scene() const { return scene; }

void forward_pass::set_apply_ambient(bool apply_ambient)
{
    this->apply_ambient = apply_ambient;
}

bool forward_pass::get_apply_ambient() const
{
    return apply_ambient;
}

void forward_pass::render_opaque(bool opaque)
{
    this->opaque = opaque;
}

void forward_pass::render_transparent(bool transparent)
{
    this->transparent = transparent;
}

std::string forward_pass::get_name() const
{
    return "forward_pass";
}

} // namespace lt::method
