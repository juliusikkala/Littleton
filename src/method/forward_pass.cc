#include "forward_pass.hh"
#include "context.hh"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

// Make sure that the shadow maps are not overridden by material textures.
// The last texture index set by 'material' is 6, so start shadow maps
// from 7.
static constexpr int SHADOW_MAP_INDEX_OFFSET = 7;

method::forward_pass::forward_pass(
    render_target& target,
    shader_store& store,
    render_scene* scene
): target_method(target),
   forward_shader(store.get(shader::path{"generic.vert", "forward.frag"})),
   scene(scene) {}

method::forward_pass::~forward_pass() {}

template<typename L, typename T>
static int find_shadow_map_index(
    L* light,
    std::vector<T*> shadow_maps
){
    for(unsigned i = 0; i < shadow_maps.size(); ++i)
    {
        if(shadow_maps[i]->get_light() == light)
        {
            return i;
        }
    }
    return -1;
}

static std::unique_ptr<uniform_block> create_light_block(
    const std::string& block_name,
    render_scene* scene,
    shader* compatible_shader,
    const glm::mat4& v,
    const std::vector<directional_shadow_map*>& directional_shadow_maps
){
    std::unique_ptr<uniform_block> light_block(
        new uniform_block(compatible_shader->get_block_type(block_name))
    );

    light_block->set<int>(
        "point_light_count",
        scene->get_point_lights().size()
    );
    light_block->set<int>(
        "directional_light_count",
        scene->get_directional_lights().size()
    );
    light_block->set<int>(
        "spotlight_count",
        scene->get_spotlights().size()
    );

    unsigned i = 0;
    for(point_light* l: scene->get_point_lights())
    {
        std::string prefix = "point["+std::to_string(i)+"].";
        light_block->set(
            prefix + "color",
            l->get_color()
        );
        light_block->set(
            prefix + "position",
            glm::vec3(v * glm::vec4(l->get_global_position(), 1))
        );
        light_block->set(prefix + "shadow_map_index", -1);
        ++i;
    }

    i = 0;
    for(directional_light* l: scene->get_directional_lights())
    {
        std::string prefix = "directional["+std::to_string(i)+"].";
        light_block->set(
            prefix + "color",
            l->get_color()
        );
        light_block->set(
            prefix + "direction",
            glm::normalize(glm::vec3(v * glm::vec4(l->get_direction(), 0)))
        );
        light_block->set(
            prefix + "shadow_map_index",
            find_shadow_map_index(l, directional_shadow_maps)
        );
        ++i;
    }

    i = 0;
    for(spotlight* l: scene->get_spotlights())
    {
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
        light_block->set(prefix + "shadow_map_index", -1);
        ++i;
    }
    light_block->upload();

    return light_block;
}

// Unfortunately, uniform blocks can't contain samplers, so shadow maps have
// to be set as separate uniforms :(
static void set_shadow_map_uniforms(
    shader* s,
    glm::mat4& model,
    const std::vector<directional_shadow_map*>& directional_shadow_maps
){
    s->set<int>("shadow_map_count", directional_shadow_maps.size());

    unsigned i = 0;
    for(i = 0; i < directional_shadow_maps.size(); ++i)
    {
        directional_shadow_map* sm = directional_shadow_maps[i];

        glm::mat4 lv = sm->get_view();
        glm::mat4 lp = sm->get_projection();
        glm::mat4 lvp = lp * lv * model;

        glm::vec2 bias = sm->get_bias();

        std::string prefix =
            "shadows["
            + std::to_string(i)
            + "].";

        s->set<int>(
            prefix + "map",
            i + SHADOW_MAP_INDEX_OFFSET
        );
        s->set(prefix + "min_bias", bias.x);
        s->set(prefix + "max_bias", bias.y);
        s->set(
            prefix + "mvp",
            lvp
        );
    }
}

static void bind_shadow_map_textures(
    context* ctx,
    const std::vector<directional_shadow_map*>& directional_shadow_maps
){
    unsigned shadow_map_index = SHADOW_MAP_INDEX_OFFSET;

    unsigned max_texture_units = (*ctx)[GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS];

    for(
        unsigned i = 0;
        i < directional_shadow_maps.size() &&
        shadow_map_index < max_texture_units;
        ++i, ++shadow_map_index
    ){
        directional_shadow_map* dsm = directional_shadow_maps[i];
        dsm->get_depth().bind(shadow_map_index);
    }
}

static shader::definition_map get_scene_definitions(render_scene* scene)
{
    return {
        {"LIGHTING", ""},
        {
            "MAX_POINT_LIGHT_COUNT",
            std::to_string(next_power_of_two(scene->point_light_count()))
        },
        {
            "MAX_DIRECTIONAL_LIGHT_COUNT",
            std::to_string(next_power_of_two(scene->directional_light_count()))
        },
        {
            "MAX_SPOTLIGHT_COUNT",
            std::to_string(next_power_of_two(scene->spotlight_count()))
        },
        {
            "MAX_SHADOW_MAP_COUNT",
            std::to_string(next_power_of_two(scene->shadow_map_count()))
        }
    };
}

static shader::definition_map get_complete_definitions(
    render_scene* scene,
    model::vertex_group& group
){

    shader::definition_map definitions = get_scene_definitions(scene);
    shader::definition_map material_definitions =
        group.mat->get_definitions();
    shader::definition_map mesh_definitions =
        group.mesh->get_definitions();

    definitions.insert(
        material_definitions.begin(),
        material_definitions.end()
    );
    definitions.insert(
        mesh_definitions.begin(),
        mesh_definitions.end()
    );
    return definitions;
}

void method::forward_pass::execute()
{
    target_method::execute();

    if(!forward_shader || !scene)
        return;

    context* ctx = &forward_shader->get_context();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);

    camera* cam = scene->get_camera();
    if(!cam) return;

    glm::mat4 v = glm::inverse(cam->get_global_transform());
    glm::mat4 p = cam->get_projection();

    std::unique_ptr<uniform_block> light_block;

    auto directional_shadow_map_set = scene->get_directional_shadow_maps();

    std::vector<directional_shadow_map*> directional_shadow_maps(
        directional_shadow_map_set.begin(),
        directional_shadow_map_set.end()
    );

    bind_shadow_map_textures(ctx, directional_shadow_maps);

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

            shader* s = forward_shader->get(
                get_complete_definitions(scene, group)
            );
            s->bind();

            if(!light_block && s->block_exists("Lights"))
            {
                light_block = create_light_block(
                    "Lights",
                    scene,
                    s,
                    v,
                    directional_shadow_maps
                );
                light_block->bind(0);
            }
            if(light_block) s->set_block("Lights", 0);

            set_shadow_map_uniforms(s, m, directional_shadow_maps);

            s->set("mvp", mvp);
            s->set("m", mv);
            s->set("n_m", n_mv);

            group.mat->apply(s);
            group.mesh->draw();
        }
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

