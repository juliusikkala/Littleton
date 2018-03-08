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
static constexpr int SHADOW_MAP_INDEX_OFFSET = 7;

method::forward_pass::forward_pass(
    render_target& target,
    shader_pool& pool,
    render_scene* scene
):  target_method(target),
    forward_shader(pool.get_shader(
        shader::path{"generic.vert", "forward.frag"})
    ),
    scene(scene)
{
}

method::forward_pass::~forward_pass() {}

static bool handle_light_in_this_pass(
    light* l,
    const std::set<basic_shadow_map*>& shadow_maps,
    const std::map<light*, basic_shadow_map*>& shadows_by_light,
    std::set<light*>& unhandled_lights,
    int& shadow_map_index
){
    // Has the light been handled already?
    if(unhandled_lights.count(l) == 0) return false;

    // Is the light shadowed by a shadow map that is not handled in this pass?
    auto shadow_it = shadows_by_light.find(l);
    if(shadow_it != shadows_by_light.end())
    {
        auto map_it = shadow_maps.find(shadow_it->second);
        if(map_it == shadow_maps.end()) return false;
        shadow_map_index = std::distance(shadow_maps.begin(), map_it);
    }
    else shadow_map_index = -1;

    unhandled_lights.erase(l);
    return true;
}

static std::unique_ptr<uniform_block> create_light_block(
    const std::string& block_name,
    render_scene* scene,
    shader* compatible_shader,
    const glm::mat4& v,
    const std::set<basic_shadow_map*>& shadow_maps,
    const std::map<light*, basic_shadow_map*>& shadows_by_light,
    std::set<light*>& unhandled_lights
){
    unsigned point_light_count = 0;
    unsigned spotlight_count = 0;
    unsigned directional_light_count = 0;

    std::unique_ptr<uniform_block> light_block(
        new uniform_block(compatible_shader->get_block_type(block_name))
    );

    unsigned i = 0;
    for(point_light* l: scene->get_point_lights())
    {
        int shadow_map_index = -1;
        if(handle_light_in_this_pass(
            l,
            shadow_maps,
            shadows_by_light,
            unhandled_lights,
            shadow_map_index
        ) == false) continue;

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
        light_block->set(prefix + "shadow_map_index", shadow_map_index);
        ++i;
    }

    i = 0;
    for(directional_light* l: scene->get_directional_lights())
    {
        int shadow_map_index = -1;
        if(handle_light_in_this_pass(
            l,
            shadow_maps,
            shadows_by_light,
            unhandled_lights,
            shadow_map_index
        ) == false) continue;

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
        light_block->set(prefix + "shadow_map_index", shadow_map_index);
        ++i;
    }

    i = 0;
    for(spotlight* l: scene->get_spotlights())
    {
        int shadow_map_index = -1;
        if(handle_light_in_this_pass(
            l,
            shadow_maps,
            shadows_by_light,
            unhandled_lights,
            shadow_map_index
        ) == false) continue;

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
        light_block->set(prefix + "shadow_map_index", shadow_map_index);
        ++i;
    }

    light_block->set<int>("point_light_count", point_light_count);
    light_block->set<int>("directional_light_count", directional_light_count);
    light_block->set<int>("spotlight_count", spotlight_count);

    light_block->upload();

    return light_block;
}

static void update_scene_definitions(
    shader::definition_map& def,
    render_scene* scene,
    size_t shadow_map_count
){
    def["LIGHTING"];
    def["MAX_POINT_LIGHT_COUNT"] = std::to_string(
        next_power_of_two(scene->point_light_count()));
    def["MAX_DIRECTIONAL_LIGHT_COUNT"] = std::to_string(
        next_power_of_two(scene->directional_light_count()));
    def["MAX_SPOTLIGHT_COUNT"] = std::to_string(
        next_power_of_two(scene->spotlight_count()));
    def["MAX_SHADOW_MAP_COUNT"] = std::to_string(
        next_power_of_two(shadow_map_count));
}

void method::forward_pass::execute()
{
    target_method::execute();
    if(!forward_shader || !scene)
        return;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);

    camera* cam = scene->get_camera();
    if(!cam) return;

    glm::mat4 v = glm::inverse(cam->get_global_transform());
    glm::mat4 p = cam->get_projection();

    const shadow_scene::shadow_map_map& shadow_maps = scene->get_shadow_maps();

    std::set<light*> unhandled_lights = scene->get_lights();
    std::map<light*, basic_shadow_map*> shadows_by_light =
        scene->get_shadow_maps_by_light();

    // Loop through shadow map implementations (separated by shared resources
    // in pair.first)
    for(auto& pair: shadow_maps)
    {
        shadow_map_impl* impl = pair.first.get();
        const std::set<basic_shadow_map*>& shadow_maps = pair.second;

        shader::definition_map shadow_map_definitions =
            impl->get_definitions();

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

                shader::definition_map def = shadow_map_definitions;
                update_scene_definitions(def, scene, shadow_maps.size());
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
                        shadow_maps,
                        shadows_by_light,
                        unhandled_lights
                    );
                    light_block->bind(0);
                }
                if(light_block) s->set_block("Lights", 0);

                unsigned texture_index = SHADOW_MAP_INDEX_OFFSET;
                s->set<int>("shadow_map_count", shadow_maps.size());
                impl->set_common_uniforms(s, texture_index);

                unsigned i = 0;
                for(basic_shadow_map* sm: shadow_maps)
                {
                    std::string prefix =
                        "shadows["
                        + std::to_string(i++)
                        + "].";

                    impl->set_shadow_map_uniforms(
                        s,
                        texture_index,
                        sm,
                        prefix,
                        m
                    );
                }

                s->set("mvp", mvp);
                s->set("m", mv);
                s->set("n_m", n_mv);

                group.mat->apply(s);
                group.mesh->draw();
            }
        }

        // The rest of the passes should be added on top of the current one.
        glEnable(GL_BLEND);
    }
}

void method::forward_pass::set_shader(multishader* s)
{
    forward_shader = s;
}

multishader* method::forward_pass::get_shader() const
{
    return forward_shader;
}

void method::forward_pass::set_scene(render_scene* s) { scene = s; }
render_scene* method::forward_pass::get_scene() const { return scene; }

std::string method::forward_pass::get_name() const
{
    return "forward_pass";
}
