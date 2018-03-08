#include "lighting_pass.hh"
#include "multishader.hh"
#include "camera.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include "shader_pool.hh"
#include "scene.hh"
#include "shadow/shadow_map.hh"

method::lighting_pass::lighting_pass(
    render_target& target,
    gbuffer& buf,
    shader_pool& pool,
    render_scene* scene
):  target_method(target), buf(&buf),
    lighting_shader(pool.get_shader(
        shader::path{"lighting.vert", "lighting.frag"})
    ),
    scene(scene),
    fullscreen_quad(vertex_buffer::create_square(target.get_context())),
    gbuf_sampler(
        target.get_context(),
        GL_NEAREST,
        GL_NEAREST,
        GL_CLAMP_TO_EDGE
    )
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

static void set_point_light(
    shader* s,
    point_light* light,
    const glm::mat4& view,
    const glm::vec4& perspective_data,
    int shadow_map_index = -1
){
    s->set("light.shadow_map_index", shadow_map_index);
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

    s->set("light.shadow_map_index", -1);
}

static void set_spotlight(
    shader* s,
    spotlight* light,
    const glm::mat4& view,
    const glm::vec4& perspective_data,
    int shadow_map_index = -1
){
    s->set("light.shadow_map_index", shadow_map_index);

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
    s->set("light.shadow_map_index", -1);
}

static void set_directional_light(
    shader* s,
    directional_light* light,
    const glm::mat4& view,
    const glm::vec4& perspective_data,
    int shadow_map_index = -1
){
    s->set("light.shadow_map_index", shadow_map_index);
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

// Returns the handled lights
static std::set<light*> render_shadowed_lights(
    multishader* lighting_shader,
    render_scene* scene,
    const glm::mat4& view,
    const glm::vec4& perspective_data,
    vertex_buffer& fullscreen_quad
){
    std::set<light*> shadowed_lights;
    for(auto& pair: scene->get_shadow_maps())
    {
        shader::definition_map default_definitions(
            pair.first->get_definitions()
        );

        for(basic_shadow_map* sm: pair.second)
        {
            directional_shadow_map* dsm =
                dynamic_cast<directional_shadow_map*>(sm);

            shader::definition_map definitions(default_definitions);
            unsigned texture_index = 4;

            if(dsm) definitions["DIRECTIONAL_LIGHT"];

            shader* s = lighting_shader->get(definitions);
            s->bind();

            pair.first->set_common_uniforms(s, texture_index);
            pair.first->set_shadow_map_uniforms(
                s,
                texture_index,
                sm,
                "shadow.",
                glm::inverse(view)
            );

            if(dsm)
            {
                directional_light* l = dsm->get_light();
                if(scene->get_directional_lights().count(l) == 0) continue;

                set_directional_light(
                    s,
                    l,
                    view,
                    perspective_data,
                    0
                );
                shadowed_lights.insert(l);
            }

            fullscreen_quad.draw();
        }
    }
    return shadowed_lights;
}

// TODO: Refactor this function into something a bit shorter and clearer.
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

    shader::definition_map point_light_definitions{{"POINT_LIGHT", ""}};
    shader::definition_map directional_light_definitions{
        {"DIRECTIONAL_LIGHT", ""}
    };
    shader::definition_map spotlight_definitions{{"SPOTLIGHT", ""}};

    gbuf_sampler.bind(buf->get_depth_stencil().bind(0));
    gbuf_sampler.bind(buf->get_color_emission().bind(1));
    gbuf_sampler.bind(buf->get_normal().bind(2));
    gbuf_sampler.bind(buf->get_material().bind(3));

    float near, far, fov, aspect;
    decompose_perspective(p, near, far, fov, aspect);
    glm::vec4 perspective_data = glm::vec4(
        2*tan(fov/2.0f)*aspect,
        2*tan(fov/2.0f),
        near,
        far
    );

    // Render shadowed lights first
    std::set<light*> shadowed_lights = render_shadowed_lights(
        lighting_shader,
        scene,
        v,
        perspective_data,
        fullscreen_quad
    );

    // Render point lights
    shader* pls = lighting_shader->get(point_light_definitions);
    pls->bind();

    for(point_light* l: scene->get_point_lights())
    {
        if(shadowed_lights.count(l)) continue;
        set_point_light(pls, l, v, perspective_data);
        fullscreen_quad.draw();
    }

    // Render spotlights
    shader* sls = lighting_shader->get(spotlight_definitions);
    sls->bind();

    for(spotlight* l: scene->get_spotlights())
    {
        if(shadowed_lights.count(l)) continue;
        set_spotlight(sls, l, v, perspective_data);
        fullscreen_quad.draw();
    }

    // Render directional lights
    shader* dls = lighting_shader->get(directional_light_definitions);
    dls->bind();

    for(directional_light* l: scene->get_directional_lights())
    {
        if(shadowed_lights.count(l)) continue;
        set_directional_light(dls, l, v, perspective_data);
        fullscreen_quad.draw();
    }
}

std::string method::lighting_pass::get_name() const
{
    return "lighting_pass";
}
