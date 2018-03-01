#include "lighting_pass.hh"

method::lighting_pass::lighting_pass(
    render_target& target,
    gbuffer& buf,
    shader_store& store,
    render_scene* scene
): target_method(target), buf(&buf),
   lighting_shader(store.get(shader::path{"lighting.vert", "lighting.frag"})),
   scene(scene),
   fullscreen_quad(vertex_buffer::create_square(target.get_context()))
{
    shadow_noise.reset(generate_shadow_noise_texture(
        target.get_context(),
        glm::uvec2(128)
    ));
    set_shadow();
}

void method::lighting_pass::set_scene(render_scene* scene)
{
    this->scene = scene;
}

render_scene* method::lighting_pass::get_scene() const
{
    return scene;
}

void method::lighting_pass::set_shadow(unsigned samples, float radius)
{
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

    shader::definition_map default_definitions({
        {"SHADOW_MAP_KERNEL_SIZE", std::to_string(shadow_kernel.size())},
        {"SHADOW_IMPLEMENTATION", "shadow/pcf.glsl"}
    });

    shader::definition_map point_light_definitions(default_definitions);
    point_light_definitions["POINT_LIGHT"];
    shader::definition_map directional_light_definitions(default_definitions);
    directional_light_definitions["DIRECTIONAL_LIGHT"];
    shader::definition_map spotlight_definitions(default_definitions);
    spotlight_definitions["SPOTLIGHT"];

    buf->get_depth_stencil().bind(0);
    buf->get_color_emission().bind(1);
    buf->get_normal().bind(2);
    buf->get_material().bind(3);
    shadow_noise->bind(4);

    float near, far, fov, aspect;
    decompose_perspective(p, near, far, fov, aspect);
    glm::vec4 perspective_data = glm::vec4(
        2*tan(fov/2.0f)*aspect,
        2*tan(fov/2.0f),
        near,
        far
    );

    // Render point lights
    shader* pls = lighting_shader->get(point_light_definitions);
    pls->bind();

    pls->set("in_depth", 0);
    pls->set("in_color_emission", 1);
    pls->set("in_normal", 2);
    pls->set("in_material", 3);
    pls->set("shadow_noise", 4);
    pls->set<glm::vec2>(
        "shadow_sample_offsets",
        shadow_kernel.size(),
        shadow_kernel.data()
    );
    pls->set("perspective_data", perspective_data);

    for(point_light* l: scene->get_point_lights())
    {
        pls->set(
            "light.position",
            glm::vec3(v * glm::vec4(l->get_global_position(), 1))
        );
        pls->set("light.color", l->get_color());

        pls->set("light.shadow_map_index", -1);

        fullscreen_quad.draw();
    }

    // Render spotlights
    shader* sls = lighting_shader->get(spotlight_definitions);
    sls->bind();

    sls->set("in_depth", 0);
    sls->set("in_color_emission", 1);
    sls->set("in_normal", 2);
    sls->set("in_material", 3);
    sls->set("shadow_noise", 4);
    sls->set<glm::vec2>(
        "shadow_sample_offsets",
        shadow_kernel.size(),
        shadow_kernel.data()
    );
    sls->set("perspective_data", perspective_data);

    for(spotlight* l: scene->get_spotlights())
    {
        sls->set(
            "light.position",
            glm::vec3(v * glm::vec4(l->get_global_position(), 1))
        );
        sls->set("light.color", l->get_color());

        sls->set(
            "light.direction",
            glm::normalize(glm::vec3(v * glm::vec4(l->get_global_direction(), 0)))
        );
        
        sls->set<float>(
            "light.cutoff",
            cos(glm::radians(l->get_cutoff_angle()))
        );
        sls->set(
            "light.exponent",
            l->get_falloff_exponent()
        );
        sls->set("light.shadow_map_index", -1);

        fullscreen_quad.draw();
    }

    // Render directional lights
    shader* dls = lighting_shader->get(directional_light_definitions);

    std::map<directional_light*, directional_shadow_map*>
    directional_shadow_maps = scene->get_directional_shadow_maps_by_light();

    dls->bind();

    dls->set("in_depth", 0);
    dls->set("in_color_emission", 1);
    dls->set("in_normal", 2);
    dls->set("in_material", 3);
    dls->set("shadow_noise", 4);
    dls->set<glm::vec2>(
        "shadow_kernel",
        shadow_kernel.size(),
        shadow_kernel.data()
    );
    dls->set("perspective_data", perspective_data);

    for(directional_light* l: scene->get_directional_lights())
    {
        auto shadow_it = directional_shadow_maps.find(l);
        if(shadow_it != directional_shadow_maps.end())
        {
            directional_shadow_map* sm = shadow_it->second;
            glm::mat4 lv = sm->get_view();
            glm::mat4 lp = sm->get_projection();
            glm::mat4 lvp = lp * lv;

            glm::vec2 bias = sm->get_bias();

            dls->set("light.shadow_map_index", 0);
            dls->set("shadow.map", sm->get_depth().bind(5));
            dls->set("shadow.min_bias", bias.x);
            dls->set("shadow.max_bias", bias.y);
            dls->set(
                "shadow.mvp",
                lvp * glm::inverse(v)
            );
        }
        else
        {
            dls->set("light.shadow_map_index", -1);
        }
        dls->set("light.color", l->get_color());
        dls->set(
            "light.direction",
            glm::normalize(glm::vec3(v * glm::vec4(l->get_direction(), 0)))
        );

        fullscreen_quad.draw();
    }
}

