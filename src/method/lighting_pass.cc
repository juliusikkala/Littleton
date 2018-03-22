#include "lighting_pass.hh"
#include "multishader.hh"
#include "camera.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include "resource_pool.hh"
#include "scene.hh"
#include "common_resources.hh"
#include "shadow_map.hh"
#include "shadow_method.hh"

method::lighting_pass::lighting_pass(
    render_target& target,
    gbuffer& buf,
    resource_pool& pool,
    render_scene* scene,
    bool apply_ambient
):  target_method(target), buf(&buf),
    lighting_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "lighting.frag"})
    ),
    scene(scene), apply_ambient(apply_ambient),
    quad(common::ensure_quad_vertex_buffer(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool))
{
}

void method::lighting_pass::set_scene(render_scene* scene)
{
    this->scene = scene;
}

render_scene* method::lighting_pass::get_scene() const
{
    return scene;
}

void method::lighting_pass::set_apply_ambient(bool apply_ambient)
{
    this->apply_ambient = apply_ambient;
}

bool method::lighting_pass::get_apply_ambient() const
{
    return apply_ambient;
}

static void set_gbuf(shader* s, const camera* cam)
{
    s->set("in_depth", 0);
    s->set("in_color_emission", 1);
    s->set("in_normal", 2);
    s->set("in_material", 3);
    s->set("projection_info", cam->get_projection_info());
    s->set("clip_info", cam->get_clip_info());
}

static void set_light(shader* s, point_light* light, const glm::mat4& view)
{
    s->set("inv_view", glm::inverse(view));
    s->set(
        "light.position",
        glm::vec3(view * glm::vec4(light->get_global_position(), 1))
    );
    s->set("light.color", light->get_color());
}

static void set_light(shader* s, spotlight* light, const glm::mat4& view)
{
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

template<typename L>
static void render_shadowed(
    multishader* lighting_shader,
    const shadow_scene::omni_map& shadows,
    const shader::definition_map& light_definitions,
    const std::vector<L*>& lights,
    std::vector<bool>& handled_lights,
    const camera* cam,
    const glm::mat4& v,
    const vertex_buffer& quad
){
    for(const auto& pair: shadows)
    {
        method::shadow_method* m = pair.first;
        shader::definition_map def(m->get_omni_definitions());

        def.insert(light_definitions.begin(), light_definitions.end());

        shader* s = lighting_shader->get(def);
        s->bind();

        unsigned texture_index = 4;
        m->set_omni_uniforms(s, texture_index);

        for(omni_shadow_map* sm: pair.second)
        {
            unsigned local_texture_index = texture_index;
            L* light = static_cast<L*>(sm->get_light());

            // Check if the light is in the scene and mark it as handled
            auto it = std::lower_bound(lights.begin(), lights.end(), light);
            if(it == lights.end() || *it != light) continue;
            handled_lights[it - lights.begin()] = true;

            m->set_shadow_map_uniforms(
                s, local_texture_index, sm, "shadow.", glm::inverse(v)
            );

            set_gbuf(s, cam);
            set_light(s, light, v);

            quad.draw();
        }
    }
}

template<typename L>
static void render_shadowed(
    multishader* lighting_shader,
    const shadow_scene::perspective_map& shadows,
    const shader::definition_map& light_definitions,
    const std::vector<L*>& lights,
    std::vector<bool>& handled_lights,
    const camera* cam,
    const glm::mat4& v,
    const vertex_buffer& quad
){
    for(const auto& pair: shadows)
    {
        method::shadow_method* m = pair.first;
        shader::definition_map def(m->get_perspective_definitions());

        def.insert(light_definitions.begin(), light_definitions.end());

        shader* s = lighting_shader->get(def);
        s->bind();

        unsigned texture_index = 4;
        m->set_omni_uniforms(s, texture_index);

        for(perspective_shadow_map* sm: pair.second)
        {
            unsigned local_texture_index = texture_index;
            L* light = static_cast<L*>(sm->get_light());

            // Check if the light is in the scene and mark it as handled
            auto it = std::lower_bound(lights.begin(), lights.end(), light);
            if(it == lights.end() || *it != light) continue;
            handled_lights[it - lights.begin()] = true;

            m->set_shadow_map_uniforms(
                s, local_texture_index, sm, "shadow.", glm::inverse(v)
            );

            set_gbuf(s, cam);
            set_light(s, light, v);

            quad.draw();
        }
    }
}


static void render_point_lights(
    multishader* lighting_shader,
    render_scene* scene,
    const glm::mat4& v,
    const vertex_buffer& quad
){
    const std::vector<point_light*>& lights = 
        scene->get_point_lights();
    std::vector<bool> handled_lights(lights.size(), false);

    shader::definition_map definitions({{"POINT_LIGHT", ""}});

    // Render shadowed lights
    render_shadowed(
        lighting_shader,
        scene->get_omni_shadows(),
        definitions,
        lights,
        handled_lights,
        scene->get_camera(),
        v, quad
    );

    render_shadowed(
        lighting_shader,
        scene->get_perspective_shadows(),
        definitions,
        lights,
        handled_lights,
        scene->get_camera(),
        v, quad
    );

    // Render unshadowed lights
    shader* s = lighting_shader->get(definitions);
    s->bind();
    set_gbuf(s, scene->get_camera());

    for(unsigned i = 0; i < lights.size(); ++i)
    {
        if(handled_lights[i]) continue;
        set_light(s, lights[i], v);
        quad.draw();
    }
}

static void render_spotlights(
    multishader* lighting_shader,
    render_scene* scene,
    const glm::mat4& v,
    const vertex_buffer& quad
){
    const std::vector<spotlight*>& lights = scene->get_spotlights();
    std::vector<bool> handled_lights(lights.size(), false);

    shader::definition_map definitions({{"SPOTLIGHT", ""}});

    // Render shadowed lights
    render_shadowed(
        lighting_shader,
        scene->get_omni_shadows(),
        definitions,
        lights,
        handled_lights,
        scene->get_camera(),
        v, quad
    );

    render_shadowed(
        lighting_shader,
        scene->get_perspective_shadows(),
        definitions,
        lights,
        handled_lights,
        scene->get_camera(),
        v, quad
    );

    // Render unshadowed lights
    shader* s = lighting_shader->get({{"SPOTLIGHT", ""}});
    s->bind();
    set_gbuf(s, scene->get_camera());

    for(unsigned i = 0; i < lights.size(); ++i)
    {
        if(handled_lights[i]) continue;
        set_light(s, lights[i], v);
        quad.draw();
    }
}

static void render_directional_lights(
    multishader* lighting_shader,
    render_scene* scene,
    const glm::mat4& v,
    const vertex_buffer& quad
){
    const std::vector<directional_light*>& lights = 
        scene->get_directional_lights();
    std::vector<bool> handled_lights(lights.size(), false);

    shader::definition_map definitions({{"DIRECTIONAL_LIGHT", ""}});

    // Render shadowed lights
    for(const auto& pair: scene->get_directional_shadows())
    {
        method::shadow_method* m = pair.first;
        shader::definition_map def(m->get_directional_definitions());

        def.insert(definitions.begin(), definitions.end());

        shader* s = lighting_shader->get(def);
        s->bind();

        unsigned texture_index = 4;
        m->set_directional_uniforms(s, texture_index);

        for(directional_shadow_map* sm: pair.second)
        {
            unsigned local_texture_index = texture_index;
            directional_light* light = sm->get_light();

            // Check if the light is in the scene and mark it as handled
            auto it = std::lower_bound(lights.begin(), lights.end(), light);
            if(it == lights.end() || *it != light) continue;
            handled_lights[it - lights.begin()] = true;

            m->set_shadow_map_uniforms(
                s, local_texture_index, sm, "shadow.", glm::inverse(v)
            );

            set_gbuf(s, scene->get_camera());
            set_light(s, light, v);

            quad.draw();
        }
    }

    // Render unshadowed lights
    shader* s = lighting_shader->get(definitions);
    s->bind();
    set_gbuf(s, scene->get_camera());

    for(unsigned i = 0; i < lights.size(); ++i)
    {
        if(handled_lights[i]) continue;
        set_light(s, lights[i], v);
        quad.draw();
    }
}

static void render_emission(
    multishader* lighting_shader,
    const vertex_buffer& quad,
    const glm::vec3& ambient,
    const camera* cam
){
    shader::definition_map def({{"EMISSION", ""}});

    shader* s = lighting_shader->get(def);
    s->bind();

    set_gbuf(s, cam);
    s->set("ambient", ambient);

    quad.draw();
}

void method::lighting_pass::execute()
{
    target_method::execute();

    if(!lighting_shader || !scene)
        return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDisable(GL_STENCIL_TEST);

    camera* cam = scene->get_camera();
    if(!cam) return;

    glm::mat4 v = glm::inverse(cam->get_global_transform());

    fb_sampler.bind(buf->get_depth_stencil(), 0);
    fb_sampler.bind(buf->get_color_emission(), 1);
    fb_sampler.bind(buf->get_normal(), 2);
    fb_sampler.bind(buf->get_material(), 3);

    render_point_lights(
        lighting_shader,
        scene,
        v,
        quad
    );

    render_spotlights(
        lighting_shader,
        scene,
        v,
        quad
    );

    render_directional_lights(
        lighting_shader,
        scene,
        v,
        quad
    );

    render_emission(
        lighting_shader,
        quad,
        apply_ambient ? scene->get_ambient() : glm::vec3(0),
        scene->get_camera()
    );
}

std::string method::lighting_pass::get_name() const
{
    return "lighting_pass";
}
