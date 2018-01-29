#include "forward_render.hh"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

method::forward_render::forward_render(
    shader_cache* forward_shader,
    scene* render_scene
): forward_shader(forward_shader), render_scene(render_scene) {}

method::forward_render::~forward_render() {}

static std::unique_ptr<uniform_block> create_light_block(
    const std::string& block_name,
    shader* compatible_shader
){
    std::unique_ptr<uniform_block> light_block(
        new uniform_block(compatible_shader->get_block_type(block_name))
    );

    light_block->set("test", glm::vec3(1.0, 1.0, 1.0));
    light_block->upload();

    return light_block;
}

void method::forward_render::execute()
{
    if(!forward_shader || !render_scene)
        return;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    camera* cam = render_scene->get_camera();
    if(!cam) return;

    glm::mat4 v = glm::inverse(cam->get_global_transform());
    glm::mat4 p = cam->get_projection();
    glm::mat4 vp = p * v;

    std::unique_ptr<uniform_block> light_block;

    for(object* obj: *render_scene)
    {
        model* mod = obj->get_model();
        if(!mod) continue;

        glm::mat4 m = obj->get_global_transform();
        glm::mat3 n_mv(v * glm::inverseTranspose(m));
        glm::mat4 mvp = vp * m;

        for(model::vertex_group& group: *mod)
        {
            if(!group.mat || !group.mesh) continue;

            shader::definition_map definitions = group.mat->get_definitions();
            shader::definition_map tmp = group.mesh->get_definitions();
            definitions.insert(tmp.begin(), tmp.end());

            shader* s = forward_shader->get(definitions);
            s->bind();

            if(!light_block) light_block = create_light_block("Lights", s);
            light_block->bind(0);

            s->set_block("Lights", 0);

            s->set("mvp", mvp);
            s->set("n_mv", n_mv);

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

void method::forward_render::set_scene(scene* s) { render_scene = s; }
scene* method::forward_render::get_scene() const { return render_scene; }

