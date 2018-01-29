#include "forward_render.hh"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

method::forward_render::forward_render(
    shader_cache* forward_shader,
    render_scene* scene
): forward_shader(forward_shader), scene(scene) {}

method::forward_render::~forward_render() {}

static std::unique_ptr<uniform_block> create_light_block(
    const std::string& block_name,
    light_scene* scene,
    shader* compatible_shader
){
    std::unique_ptr<uniform_block> light_block(
        new uniform_block(compatible_shader->get_block_type(block_name))
    );

    std::vector<glm::vec3> colors;
    std::vector<glm::vec3> positions;

    for(point_light* l: scene->get_point_lights())
    {
        colors.push_back(l->get_color());
        positions.push_back(l->get_global_position());
    }

    light_block->set("colors", colors.size(), colors.data());
    light_block->set("positions", positions.size(), positions.data());
    light_block->upload();

    return light_block;
}

void method::forward_render::execute()
{
    if(!forward_shader || !scene)
        return;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    camera* cam = scene->get_camera();
    if(!cam) return;

    glm::mat4 v = glm::inverse(cam->get_global_transform());
    glm::mat4 p = cam->get_projection();
    glm::mat4 vp = p * v;

    std::unique_ptr<uniform_block> light_block;
    shader::definition_map scene_definitions({
        {"LIGHT_COUNT", std::to_string(scene->point_light_count())}
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

void method::forward_render::set_shader_cache(shader_cache* s)
{
    forward_shader = s;
}

shader_cache* method::forward_render::get_shader_cache() const
{
    return forward_shader;
}

void method::forward_render::set_scene(render_scene* s) { scene = s; }
render_scene* method::forward_render::get_scene() const { return scene; }

