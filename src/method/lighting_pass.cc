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
    std::vector<shadow_method*>&& shadows
):  target_method(target), buf(&buf),
    lighting_shader(pool.get_shader(
        shader::path{"lighting.vert", "lighting.frag"})
    ),
    scene(scene), shadows(std::move(shadows)),
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

static void set_light(
    shader* s,
    point_light* light,
    const glm::mat4& view,
    const glm::vec4& perspective_data
){
    s->set("in_depth", 0);
    s->set("in_color_emission", 1);
    s->set("in_normal", 2);
    s->set("in_material", 3);
    s->set("perspective_data", perspective_data);
    s->set(
        "light.position",
        glm::vec3(view * glm::vec4(light->get_global_position(), 1))
    );
    s->set("light.color", light->get_color());
}

static void set_light(
    shader* s,
    spotlight* light,
    const glm::mat4& view,
    const glm::vec4& perspective_data
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
    
    s->set("in_depth", 0);
    s->set("in_color_emission", 1);
    s->set("in_normal", 2);
    s->set("in_material", 3);
    s->set("perspective_data", perspective_data);
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
    const glm::mat4& view,
    const glm::vec4& perspective_data
){
    s->set("in_depth", 0);
    s->set("in_color_emission", 1);
    s->set("in_normal", 2);
    s->set("in_material", 3);
    s->set("perspective_data", perspective_data);
    s->set("light.color", light->get_color());
    s->set(
        "light.direction",
        glm::normalize(glm::vec3(view * glm::vec4(light->get_direction(), 0)))
    );
}

static void render_point_lights(
    multishader* lighting_shader,
    render_scene* scene,
    const glm::mat4& v,
    const glm::vec4& perspective_data,
    const vertex_buffer& quad
){
    // Render unshadowed lights
    shader* s = lighting_shader->get({{"POINT_LIGHT", ""}});
    s->bind();

    for(point_light* l: scene->get_point_lights())
    {
        set_light(s, l, v, perspective_data);
        quad.draw();
    }
}

static void render_spotlights(
    multishader* lighting_shader,
    render_scene* scene,
    const glm::mat4& v,
    const glm::vec4& perspective_data,
    const vertex_buffer& quad
){
    // Render unshadowed lights
    shader* s = lighting_shader->get({{"SPOTLIGHT", ""}});
    s->bind();

    for(spotlight* l: scene->get_spotlights())
    {
        set_light(s, l, v, perspective_data);
        quad.draw();
    }
}

static void render_directional_lights(
    multishader* lighting_shader,
    render_scene* scene,
    const std::vector<method::shadow_method*>& shadows,
    const glm::mat4& v,
    const glm::vec4& perspective_data,
    const vertex_buffer& quad
){
    const std::vector<directional_light*>& lights = 
        scene->get_directional_lights();
    std::vector<bool> handled_lights(lights.size(), false);

    shader::definition_map definitions({{"DIRECTIONAL_LIGHT", ""}});

    // Render shadowed lights
    for(method::shadow_method* m: shadows)
    {
        shader::definition_map def(m->get_directional_definitions());

        def.insert(definitions.begin(), definitions.end());

        shader* s = lighting_shader->get(def);
        s->bind();

        unsigned texture_index = 4;
        m->set_directional_uniforms(s, texture_index);

        for(unsigned i = 0; i < m->get_directional_shadow_map_count(); ++i)
        {
            unsigned local_texture_index = texture_index;
            directional_light* light =
                m->get_directional_shadow_map(i)->get_light();

            // Check if the light is in the scene and mark it as handled
            auto it = std::lower_bound(lights.begin(), lights.end(), light);
            if(it == lights.end() || *it != light) continue;
            handled_lights[it - lights.begin()] = true;

            m->set_directional_shadow_map_uniforms(
                s, local_texture_index, i, "shadow.", glm::inverse(v)
            );

            set_light(s, light, v, perspective_data);

            quad.draw();
        }
    }

    // Render unshadowed lights
    shader* s = lighting_shader->get(definitions);
    s->bind();

    for(unsigned i = 0; i < lights.size(); ++i)
    {
        if(handled_lights[i]) continue;
        set_light(s, lights[i], v, perspective_data);
        quad.draw();
    }
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
    glm::mat4 p = cam->get_projection();

    fb_sampler.bind(buf->get_depth_stencil(), 0);
    fb_sampler.bind(buf->get_color_emission(), 1);
    fb_sampler.bind(buf->get_normal(), 2);
    fb_sampler.bind(buf->get_material(), 3);

    float near, far, fov, aspect;
    decompose_perspective(p, near, far, fov, aspect);
    glm::vec4 perspective_data = glm::vec4(
        2*tan(fov/2.0f)*aspect,
        2*tan(fov/2.0f),
        near,
        far
    );

    render_point_lights(
        lighting_shader,
        scene,
        v,
        perspective_data,
        quad
    );
    render_spotlights(
        lighting_shader,
        scene,
        v,
        perspective_data,
        quad
    );
    render_directional_lights(
        lighting_shader,
        scene,
        shadows,
        v,
        perspective_data,
        quad
    );
}

std::string method::lighting_pass::get_name() const
{
    return "lighting_pass";
}
