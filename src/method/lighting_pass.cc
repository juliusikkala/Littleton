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
{}

void method::lighting_pass::set_scene(render_scene* scene)
{
    this->scene = scene;
}

render_scene* method::lighting_pass::get_scene() const
{
    return scene;
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

    shader::definition_map point_light_definitions({{"POINT_LIGHT", ""}});
    shader::definition_map directional_light_definitions(
        {{"DIRECTIONAL_LIGHT", ""}}
    );
    shader::definition_map spotlight_definitions(
        {{"SPOTLIGHT", ""}}
    );

    buf->get_depth_stencil().bind(0);
    buf->get_color_emission().bind(1);
    buf->get_normal().bind(2);
    buf->get_material().bind(3);

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

    pls->set("in_depth_stencil", 0);
    pls->set("in_color_emission", 1);
    pls->set("in_normal", 2);
    pls->set("in_material", 3);
    pls->set("perspective_data", perspective_data);

    for(point_light* l: scene->get_point_lights())
    {
        pls->set(
            "light.position",
            glm::vec3(v * glm::vec4(l->get_global_position(), 1))
        );
        pls->set("light.color", l->get_color());

        fullscreen_quad.draw();
    }

    // Render spotlights
    shader* sls = lighting_shader->get(spotlight_definitions);
    sls->bind();

    sls->set("in_depth_stencil", 0);
    sls->set("in_color_emission", 1);
    sls->set("in_normal", 2);
    sls->set("in_material", 3);
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

        fullscreen_quad.draw();
    }

    // Render directional lights
    shader* dls = lighting_shader->get(directional_light_definitions);
    dls->bind();

    dls->set("in_depth_stencil", 0);
    dls->set("in_color_emission", 1);
    dls->set("in_normal", 2);
    dls->set("in_material", 3);

    for(directional_light* l: scene->get_directional_lights())
    {
        dls->set(
            "light.color",
            l->get_color()
        );
        dls->set(
            "light.direction",
            glm::normalize(glm::vec3(v * glm::vec4(l->get_direction(), 0)))
        );

        fullscreen_quad.draw();
    }
}

