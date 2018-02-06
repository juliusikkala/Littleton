#include "forward_pass.hh"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

method::forward_pass::forward_pass(
    render_target& target,
    multishader* forward_shader,
    render_scene* scene
): pipeline_method(target), forward_shader(forward_shader), scene(scene) {}

method::forward_pass::~forward_pass() {}

static std::unique_ptr<uniform_block> create_light_block(
    const std::string& block_name,
    light_scene* scene,
    shader* compatible_shader
){
    std::unique_ptr<uniform_block> light_block(
        new uniform_block(compatible_shader->get_block_type(block_name))
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
            l->get_global_position()
        );
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
            glm::normalize(l->get_direction())
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
            l->get_global_position()
        );
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
        ++i;
    }
    light_block->upload();

    return light_block;
}

void method::forward_pass::execute()
{
    if(!forward_shader || !scene)
        return;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    camera* cam = scene->get_camera();
    if(!cam) return;

    glm::mat4 v = glm::inverse(cam->get_global_transform());
    glm::mat4 p = cam->get_projection();
    glm::mat4 vp = p * v;

    std::unique_ptr<uniform_block> light_block;
    shader::definition_map scene_definitions({
        {"LIGHTING", ""},
        {"POINT_LIGHT_COUNT", std::to_string(scene->point_light_count())},
        {"DIRECTIONAL_LIGHT_COUNT",
         std::to_string(scene->directional_light_count())},
        {"SPOTLIGHT_COUNT", std::to_string(scene->spotlight_count())}
    });

    for(object* obj: scene->get_objects())
    {
        model* mod = obj->get_model();
        if(!mod) continue;

        glm::mat4 m = obj->get_global_transform();
        glm::mat3 n_m(glm::inverseTranspose(m));
        glm::mat4 mvp = vp * m;

        for(model::vertex_group& group: *mod)
        {
            if(!group.mat || !group.mesh) continue;

            shader::definition_map definitions = scene_definitions;
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

            shader* s = forward_shader->get(definitions);
            s->bind();

            if(!light_block && s->block_exists("Lights"))
            {
                light_block = create_light_block("Lights", scene, s);
                light_block->bind(0);
            }
            if(light_block) s->set_block("Lights", 0);

            s->set("mvp", mvp);
            s->set("m", m);
            s->set("n_m", n_m);

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

