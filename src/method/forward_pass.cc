#include "forward_pass.hh"
#include "context.hh"
#include "camera.hh"
#include "model.hh"
#include "object.hh"
#include "material.hh"
#include "helpers.hh"
#include "multishader.hh"
#include "shader_pool.hh"
#include "scene.hh"
#include "vertex_buffer.hh"
#include "shadow_map.hh"
#include "shadow_method.hh"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

// Make sure that the shadow maps are not overridden by material textures.
// The last texture index set by 'material' is 6, so start shadow maps
// from 7.
// TODO: Make material report the number of used texture indices like shadow
// maps do.
static constexpr int SHADOW_MAP_INDEX_OFFSET = 7;

method::forward_pass::forward_pass(
    render_target& target,
    shader_pool& pool,
    render_scene* scene
):  target_method(target),
    forward_shader(pool.get(
        shader::path{"generic.vert", "forward.frag"})
    ),
    depth_shader(pool.get(shader::path{"generic.vert", "depth.frag"})),
    scene(scene)
{}

method::forward_pass::~forward_pass() {}

static void set_light(
    shader* s,
    point_light* light
){
    s->set("light.position", light->get_global_position());
    s->set("light.color", light->get_color());
}

static void set_light(
    shader* s,
    spotlight* light
){
    s->set("light.position", light->get_global_position());
    s->set("light.color", light->get_color());

    s->set(
        "light.direction",
        glm::normalize(light->get_global_direction())
    );
    s->set<float>(
        "light.cutoff",
        cos(glm::radians(light->get_cutoff_angle()))
    );
    s->set("light.exponent", light->get_falloff_exponent());
}

static void set_light(
    shader* s,
    directional_light* light
){
    s->set("light.color", light->get_color());
    s->set("light.direction", glm::normalize(light->get_direction()));
}

static void set_shadow(
    method::shadow_method* met,
    shader* s,
    unsigned& texture_index,
    directional_shadow_map* sm,
    const glm::mat4& m
){
    met->set_directional_uniforms(s, texture_index);
    met->set_shadow_map_uniforms(s, texture_index, sm, "shadow.", m);
}

static void set_shadow(
    method::shadow_method* met,
    shader* s,
    unsigned& texture_index,
    omni_shadow_map* sm,
    const glm::mat4& m
){
    met->set_omni_uniforms(s, texture_index);
    met->set_shadow_map_uniforms(s, texture_index, sm, "shadow.", m);
}

static void set_shadow(
    method::shadow_method* met,
    shader* s,
    unsigned& texture_index,
    perspective_shadow_map* sm,
    const glm::mat4& m
){
    met->set_perspective_uniforms(s, texture_index);
    met->set_shadow_map_uniforms(s, texture_index, sm, "shadow.", m);
}

template<typename L, typename S>
static void render_pass(
    method::shadow_method* met,
    const shader::definition_map& scene_definitions,
    render_scene* scene,
    multishader* forward_shader,
    L* light,
    S* sm
){
    camera* cam = scene->get_camera();
    glm::mat4 v = glm::inverse(cam->get_global_transform());
    glm::mat4 p = cam->get_projection();
    glm::vec3 camera_pos = cam->get_global_position();

    for(object* obj: scene->get_objects())
    {
        model* mod = obj->get_model();
        if(!mod) continue;

        glm::mat4 m = obj->get_global_transform();
        glm::mat3 n_m(glm::inverseTranspose(m));
        glm::mat4 mvp = p * v * m;

        for(model::vertex_group& group: *mod)
        {
            if(!group.mat || !group.mesh) continue;

            shader::definition_map def = scene_definitions;
            group.mat->update_definitions(def);
            group.mesh->update_definitions(def);

            shader* s = forward_shader->get(def);
            s->bind();

            unsigned texture_index = SHADOW_MAP_INDEX_OFFSET;

            set_shadow(met, s, texture_index, sm, m);
            set_light(s, light);

            s->set("mvp", mvp);
            s->set("m", m);
            s->set("n_m", n_m);
            s->set("camera_pos", camera_pos);

            group.mat->apply(s);
            group.mesh->draw();
        }
    }
}

static void render_shadowed_lights(
    multishader* forward_shader,
    std::vector<bool>& handled_point_lights,
    std::vector<bool>& handled_spotlights,
    std::vector<bool>& handled_directional_lights,
    render_scene* scene
){
    // Directional shadows are a bit simpler to use since they are always bound
    // to only one light type, directional_light.
    shader::definition_map directional_def(
        {{"DIRECTIONAL_LIGHT", ""},
         {"SINGLE_LIGHT", ""}}
    );

    const std::vector<directional_light*>& directional_lights =
        scene->get_directional_lights();

    for(const auto& pair: scene->get_directional_shadows())
    {
        method::shadow_method* met = pair.first;

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

            render_pass(
                met, scene_definitions, scene, forward_shader, light, sm
            );
        }
    }

    shader::definition_map point_def(
        {{"POINT_LIGHT", ""},
         {"SINGLE_LIGHT", ""}}
    );

    shader::definition_map spot_def(
        {{"SPOTLIGHT", ""},
         {"SINGLE_LIGHT", ""}}
    );

    const std::vector<point_light*>& point_lights = scene->get_point_lights();
    const std::vector<spotlight*>& spotlights = scene->get_spotlights();

    for(const auto& pair: scene->get_omni_shadows())
    {
        method::shadow_method* met = pair.first;

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
            {
                handled_point_lights[point_it - point_lights.begin()] = true;

                render_pass(
                    met, point_definitions, scene, forward_shader, point, sm
                );
            }
            else if(spot_it != spotlights.end() && *spot_it == spot)
            {
                handled_spotlights[spot_it - spotlights.begin()] = true;

                render_pass(
                    met, spot_definitions, scene, forward_shader, spot, sm
                );
            }
        }
    }

    for(const auto& pair: scene->get_perspective_shadows())
    {
        method::shadow_method* met = pair.first;

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

                render_pass(
                    met, point_definitions, scene, forward_shader, point, sm
                );
            }
            else if(spot_it != spotlights.end() && *spot_it == spot)
            {
                handled_spotlights[spot_it - spotlights.begin()] = true;

                render_pass(
                    met, spot_definitions, scene, forward_shader, spot, sm
                );
            }
        }
    }
}

static std::unique_ptr<uniform_block> create_light_block(
    const std::string& block_name,
    render_scene* scene,
    shader* compatible_shader,
    const std::vector<bool>& handled_point_lights,
    const std::vector<bool>& handled_spotlights,
    const std::vector<bool>& handled_directional_lights
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

        point_light_count++;
        std::string prefix = "point["+std::to_string(i)+"].";
        light_block->set(prefix + "color", l->get_color());
        light_block->set(prefix + "position", l->get_global_position());
    }

    const std::vector<spotlight*>& spotlights = scene->get_spotlights();
    for(unsigned i = 0; i < spotlights.size(); ++i)
    {
        if(handled_spotlights[i]) continue;
        spotlight* l = spotlights[i];

        spotlight_count++;
        std::string prefix = "spot["+std::to_string(i)+"].";
        light_block->set(prefix + "color", l->get_color());
        light_block->set(prefix + "position", l->get_global_position());
        light_block->set(
            prefix + "direction",
            glm::normalize(l->get_global_direction())
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

        directional_light_count++;
        std::string prefix = "directional["+std::to_string(i)+"].";
        light_block->set(
            prefix + "color",
            l->get_color()
        );
        light_block->set(
            prefix + "direction",
            glm::normalize(l->get_direction())
        );
    }

    light_block->set<int>("point_light_count", point_light_count);
    light_block->set<int>("directional_light_count", directional_light_count);
    light_block->set<int>("spotlight_count", spotlight_count);

    light_block->upload();

    return light_block;
}


static void update_scene_definitions(
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

static void render_unshadowed_lights(
    multishader* forward_shader,
    const std::vector<bool>& handled_point_lights,
    const std::vector<bool>& handled_spotlights,
    const std::vector<bool>& handled_directional_lights,
    render_scene* scene
){
    camera* cam = scene->get_camera();
    glm::mat4 v = glm::inverse(cam->get_global_transform());
    glm::mat4 p = cam->get_projection();

    shader::definition_map scene_definitions;
    update_scene_definitions(scene_definitions, scene);

    std::unique_ptr<uniform_block> light_block;

    glm::vec3 camera_pos = scene->get_camera()->get_global_position();

    for(object* obj: scene->get_objects())
    {
        model* mod = obj->get_model();
        if(!mod) continue;

        glm::mat4 m = obj->get_global_transform();
        glm::mat3 n_m(glm::inverseTranspose(m));
        glm::mat4 mvp = p * v * m;

        for(model::vertex_group& group: *mod)
        {
            if(!group.mat || !group.mesh) continue;

            shader::definition_map def = scene_definitions;
            group.mat->update_definitions(def);
            group.mesh->update_definitions(def);

            shader* s = forward_shader->get(def);
            s->bind();

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
                    handled_directional_lights
                );
                light_block->bind(0);
            }
            if(light_block) s->set_block("Lights", 0);

            s->set("mvp", mvp);
            s->set("m", m);
            s->set("n_m", n_m);
            s->set("camera_pos", camera_pos);

            group.mat->apply(s);
            group.mesh->draw();
        }
    }
}

static void depth_pass(
    multishader* depth_shader,
    render_scene* scene
){
    camera* cam = scene->get_camera();
    glm::mat4 v = glm::inverse(cam->get_global_transform());
    glm::mat4 p = cam->get_projection();

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    for(object* obj: scene->get_objects())
    {
        model* mod = obj->get_model();
        if(!mod) continue;

        glm::mat4 m = obj->get_global_transform();
        glm::mat4 mvp = p * v * m;

        for(model::vertex_group& group: *mod)
        {
            if(!group.mat || !group.mesh) continue;

            shader::definition_map def({{"DISCARD_ALPHA", "0.5"}});
            group.mat->update_definitions(def);
            group.mesh->update_definitions(def);

            shader* s = depth_shader->get(def);
            s->bind();

            s->set("mvp", mvp);
            s->set("m", m);

            group.mat->apply(s);
            group.mesh->draw();
        }
    }
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void method::forward_pass::execute()
{
    target_method::execute();
    if(!forward_shader || !scene)
        return;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);

    camera* cam = scene->get_camera();
    if(!cam) return;

    std::vector<bool> handled_point_lights(scene->point_light_count(), false);
    std::vector<bool> handled_spotlights(scene->spotlight_count(), false);
    std::vector<bool> handled_directional_lights(
        scene->directional_light_count(), false
    );

    glDisable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);

    depth_pass(depth_shader, scene);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    render_shadowed_lights(
        forward_shader,
        handled_point_lights,
        handled_spotlights,
        handled_directional_lights,
        scene
    );

    render_unshadowed_lights(
        forward_shader,
        handled_point_lights,
        handled_spotlights,
        handled_directional_lights,
        scene
    );
}

void method::forward_pass::set_scene(render_scene* s) { scene = s; }
render_scene* method::forward_pass::get_scene() const { return scene; }

std::string method::forward_pass::get_name() const
{
    return "forward_pass";
}
