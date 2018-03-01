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
   scene(scene)
{
    shadow_noise.reset(generate_shadow_noise_texture(
        target.get_context(),
        glm::uvec2(128)
    ));
    set_shadow();
}

method::forward_pass::~forward_pass() {}

template<typename L, typename T>
static int find_shadow_map_index(
    L* light,
    const std::set<T*>& shadow_maps
){
    int i = 0;
    for(auto it = shadow_maps.begin(); it != shadow_maps.end(); ++it, ++i)
    {
        if((*it)->get_light() == light)
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
    const std::set<directional_shadow_map*>& directional_shadow_maps
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
    const std::set<directional_shadow_map*>& directional_shadow_maps,
    const std::vector<glm::vec2>& shadow_kernel
){
    // Since OpenGL must be used from the main thread, no safety concerns here.
    struct shadow_map_uniform_keys
    {
        std::string map, min_bias, max_bias, mvp;
    };
    static std::vector<shadow_map_uniform_keys> key_cache;

    // Fill in the missing keys to key_cache
    if(key_cache.size() < directional_shadow_maps.size())
    {
        unsigned i = key_cache.size();
        key_cache.resize(directional_shadow_maps.size());
        for(; i < directional_shadow_maps.size(); ++i)
        {
            std::string prefix =
                "shadows["
                + std::to_string(i)
                + "].";

            key_cache[i].map = prefix + "map";
            key_cache[i].min_bias = prefix + "min_bias";
            key_cache[i].max_bias = prefix + "max_bias";
            key_cache[i].mvp = prefix + "mvp";
        }
    }

    s->set<int>("shadow_map_count", directional_shadow_maps.size());
    s->set<glm::vec2>(
        "shadow_kernel",
        shadow_kernel.size(),
        shadow_kernel.data()
    );

    unsigned i = 0;
    for(
        auto it = directional_shadow_maps.begin();
        it != directional_shadow_maps.end();
        ++it, ++i
    ){
        directional_shadow_map* sm = *it;
        shadow_map_uniform_keys& keys = key_cache[i];

        glm::mat4 lv = sm->get_view();
        glm::mat4 lp = sm->get_projection();
        glm::mat4 lvp = lp * lv * model;

        glm::vec2 bias = sm->get_bias();

        s->set<int>(keys.map, i + SHADOW_MAP_INDEX_OFFSET);
        s->set(keys.min_bias, bias.x);
        s->set(keys.max_bias, bias.y);
        s->set(keys.mvp, lvp);
    }
    s->set<int>("shadow_noise", i + SHADOW_MAP_INDEX_OFFSET);
}

static void bind_shadow_map_textures(
    context* ctx,
    texture* shadow_noise,
    const std::set<directional_shadow_map*>& directional_shadow_maps
){
    unsigned shadow_map_index = SHADOW_MAP_INDEX_OFFSET;

    unsigned max_texture_units = (*ctx)[GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS];

    for(
        auto it = directional_shadow_maps.begin();
        it != directional_shadow_maps.end() &&
        shadow_map_index < max_texture_units;
        ++it, ++shadow_map_index
    ){
        (*it)->get_depth().bind(shadow_map_index);
    }
    shadow_noise->bind(shadow_map_index);
}

static void update_scene_definitions(
    shader::definition_map& def,
    render_scene* scene,
    size_t shadow_kernel_size
){
    def["LIGHTING"];
    def["MAX_POINT_LIGHT_COUNT"] = std::to_string(
        next_power_of_two(scene->point_light_count()));
    def["MAX_DIRECTIONAL_LIGHT_COUNT"] = std::to_string(
        next_power_of_two(scene->directional_light_count()));
    def["MAX_SPOTLIGHT_COUNT"] = std::to_string(
        next_power_of_two(scene->spotlight_count()));
    def["MAX_SHADOW_MAP_COUNT"] = std::to_string(
        next_power_of_two(scene->shadow_map_count()));
    def["SHADOW_MAP_KERNEL_SIZE"] = std::to_string(shadow_kernel_size);
    def["SHADOW_IMPLEMENTATION"] = "shadow/pcf.glsl";
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

    const std::set<directional_shadow_map*>& directional_shadow_maps =
        scene->get_directional_shadow_maps();

    bind_shadow_map_textures(ctx, shadow_noise.get(), directional_shadow_maps);

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

            shader::definition_map& def = definitions_cache[&group];
            update_scene_definitions(
                def,
                scene,
                shadow_kernel.size()
            );
            group.mat->update_definitions(def);
            group.mesh->update_definitions(def);

            shader* s = forward_shader->get(def);
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

            set_shadow_map_uniforms(
                s,
                m,
                directional_shadow_maps,
                shadow_kernel
            );

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

void method::forward_pass::set_shadow(
    unsigned samples,
    float radius
){
    if(samples == 0) shadow_kernel.clear();
    else
    {
        shadow_kernel = mitchell_best_candidate(
            radius,
            20,
            samples
        );
    }
}
