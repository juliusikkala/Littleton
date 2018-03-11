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
#include "shadow/shadow_map.hh"
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
    point_light* light,
    const glm::mat4& view
){
    s->set(
        "light.position",
        glm::vec3(view * glm::vec4(light->get_global_position(), 1))
    );
    s->set("light.color", light->get_color());
}

static void set_light(
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
        glm::normalize(
            glm::vec3(
                view * glm::vec4(light->get_global_direction(), 0)
            )
        )
    );
    s->set<float>(
        "light.cutoff",
        cos(glm::radians(light->get_cutoff_angle()))
    );
    s->set(
        "light.exponent",
        light->get_falloff_exponent()
    );
}

static void set_light(
    shader* s,
    directional_light* light,
    const glm::mat4& view
){
    s->set("light.color", light->get_color());
    s->set(
        "light.direction",
        glm::normalize(glm::vec3(view * glm::vec4(light->get_direction(), 0)))
    );
}

static void update_scene_definitions(
    shader::definition_map& def,
    render_scene* scene,
    directional_light* light
){
    def["SINGLE_LIGHT"];
    def["DIRECTIONAL_LIGHT"];
}

template<typename I, typename S>
void render_shadowed_light(
    multishader* forward_shader,
    S* shadow_map,
    I* impl,
    render_scene* scene,
    const glm::mat4& v,
    const glm::mat4& p
){
    shader::definition_map scene_definitions = impl->get_definitions();
    update_scene_definitions(scene_definitions, scene, shadow_map->get_light());

    for(object* obj: scene->get_objects())
    {
        model* mod = obj->get_model();
        if(!mod) continue;

        glm::mat4 m = obj->get_global_transform();
        glm::mat4 mv = v * m;
        glm::mat3 n_mv(glm::inverseTranspose(mv));
        glm::mat4 mvp = p * mv;

        for(model::vertex_group& group: *mod)
        {
            if(!group.mat || !group.mesh) continue;

            shader::definition_map def = scene_definitions;
            group.mat->update_definitions(def);
            group.mesh->update_definitions(def);

            shader* s = forward_shader->get(def);
            s->bind();

            unsigned texture_index = SHADOW_MAP_INDEX_OFFSET;

            impl->set_common_uniforms(s, texture_index);
            impl->set_shadow_map_uniforms(
                s,
                texture_index,
                shadow_map,
                "shadow.",
                m
            );
            set_light(s, shadow_map->get_light(), v);

            s->set("mvp", mvp);
            s->set("m", mv);
            s->set("n_m", n_mv);

            group.mat->apply(s);
            group.mesh->draw();
        }
    }
}

template<typename L, typename S>
void render_shadowed_lights(
    multishader* forward_shader,
    const std::vector<L*>& lights,
    std::vector<bool>& handled_lights,
    const S& shadow_maps,
    render_scene* scene,
    const glm::mat4& v,
    const glm::mat4& p
){
    for(const auto& pair: shadow_maps)
    {
        for(auto* sm: pair.second)
        {
            L* light = sm->get_light();
            auto it = std::lower_bound(lights.begin(), lights.end(), light);
            if(it == lights.end() || *it != light) continue;
            handled_lights[it - lights.begin()] = true;

            render_shadowed_light(
                forward_shader, sm, pair.first.get(), scene, v, p
            );
        }
    }
}

static std::unique_ptr<uniform_block> create_light_block(
    const std::string& block_name,
    render_scene* scene,
    shader* compatible_shader,
    const glm::mat4& v,
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
        light_block->set(
            prefix + "color",
            l->get_color()
        );
        light_block->set(
            prefix + "position",
            glm::vec3(v * glm::vec4(l->get_global_position(), 1))
        );
    }

    const std::vector<spotlight*>& spotlights = scene->get_spotlights();
    for(unsigned i = 0; i < spotlights.size(); ++i)
    {
        if(handled_spotlights[i]) continue;
        spotlight* l = spotlights[i];

        spotlight_count++;
        std::string prefix = "spot["+std::to_string(i)+"].";
        light_block->set(
            prefix + "color",
            l->get_color()
        );
        light_block->set(
            prefix + "position",
            glm::vec3(v * glm::vec4(l->get_global_position(), 1))
        );
        light_block->set(
            prefix + "direction",
            glm::normalize(glm::vec3(
                v * glm::vec4(l->get_global_direction(), 0)))
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
            glm::normalize(glm::vec3(v * glm::vec4(l->get_direction(), 0)))
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

void render_unshadowed_lights(
    multishader* forward_shader,
    const std::vector<bool>& handled_point_lights,
    const std::vector<bool>& handled_spotlights,
    const std::vector<bool>& handled_directional_lights,
    render_scene* scene,
    const glm::mat4& v,
    const glm::mat4& p
){
    shader::definition_map scene_definitions;
    update_scene_definitions(scene_definitions, scene);

    std::unique_ptr<uniform_block> light_block;

    for(object* obj: scene->get_objects())
    {
        model* mod = obj->get_model();
        if(!mod) continue;

        glm::mat4 m = obj->get_global_transform();
        glm::mat4 mv = v * m;
        glm::mat3 n_mv(glm::inverseTranspose(mv));
        glm::mat4 mvp = p * mv;

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
                    v,
                    handled_point_lights,
                    handled_spotlights,
                    handled_directional_lights
                );
                light_block->bind(0);
            }
            if(light_block) s->set_block("Lights", 0);

            s->set("mvp", mvp);
            s->set("m", mv);
            s->set("n_m", n_mv);

            group.mat->apply(s);
            group.mesh->draw();
        }
    }
}

static void depth_pass(
    multishader* depth_shader,
    render_scene* scene,
    const glm::mat4& v,
    const glm::mat4& p
){
    for(object* obj: scene->get_objects())
    {
        model* mod = obj->get_model();
        if(!mod) continue;

        glm::mat4 m = obj->get_global_transform();
        glm::mat4 mv = v * m;
        glm::mat3 n_mv(glm::inverseTranspose(mv));
        glm::mat4 mvp = p * mv;

        for(model::vertex_group& group: *mod)
        {
            if(!group.mat || !group.mesh) continue;

            shader::definition_map def({{"DISCARD_ALPHA", "0.5"}});
            group.mat->update_definitions(def);
            group.mesh->update_definitions(def);

            shader* s = depth_shader->get(def);
            s->bind();

            s->set("mvp", mvp);
            s->set("m", mv);
            s->set("n_m", n_mv);

            group.mat->apply(s);
            group.mesh->draw();
        }
    }
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

    glm::mat4 v = glm::inverse(cam->get_global_transform());
    glm::mat4 p = cam->get_projection();

    const std::vector<point_light*>& point_lights = scene->get_point_lights();
    std::vector<bool> handled_point_lights(point_lights.size(), false);

    const std::vector<spotlight*>& spotlights = scene->get_spotlights();
    std::vector<bool> handled_spotlights(spotlights.size(), false);

    const std::vector<directional_light*>& directional_lights =
        scene->get_directional_lights();

    std::vector<bool> handled_directional_lights(
        directional_lights.size(), false
    );

    glDisable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);
    depth_pass(depth_shader, scene, v, p);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    render_shadowed_lights(
        forward_shader,
        directional_lights,
        handled_directional_lights,
        scene->get_directional_shadow_maps(),
        scene, v, p
    );

    render_unshadowed_lights(
        forward_shader,
        handled_point_lights,
        handled_spotlights,
        handled_directional_lights,
        scene, v, p
    );
}

void method::forward_pass::set_scene(render_scene* s) { scene = s; }
render_scene* method::forward_pass::get_scene() const { return scene; }

std::string method::forward_pass::get_name() const
{
    return "forward_pass";
}
