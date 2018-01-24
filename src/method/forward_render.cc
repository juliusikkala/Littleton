#include "forward_render.hh"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

method::forward_render::forward_render(
    shader_cache* forward_shader,
    scene* render_scene
): forward_shader(forward_shader), render_scene(render_scene) {}

method::forward_render::~forward_render() {}

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

