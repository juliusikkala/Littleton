#include "geometry_pass.hh"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

method::geometry_pass::geometry_pass(
    gbuffer& buf,
    shader_store& store,
    render_scene* scene
): target_method(buf),
   geometry_shader(store.get(shader::path{"generic.vert", "geometry.frag"})),
   scene(scene) {}

void method::geometry_pass::set_scene(render_scene* scene)
{
    this->scene = scene;
}

render_scene* method::geometry_pass::get_scene() const
{
    return scene;
}

void method::geometry_pass::execute()
{
    target_method::execute();
    if(!geometry_shader || !scene)
        return;

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
    glm::mat4 vp = p * v;

    for(object* obj: scene->get_objects())
    {
        model* mod = obj->get_model();
        if(!mod) continue;

        glm::mat4 m = obj->get_global_transform();
        glm::mat3 n_m(glm::inverseTranspose(v * m));
        glm::mat4 mvp = vp * m;

        for(model::vertex_group& group: *mod)
        {
            if(!group.mat || !group.mesh) continue;

            shader::definition_map& definitions = definitions_cache[&group];
            group.mat->update_definitions(definitions);
            group.mesh->update_definitions(definitions);

            shader* s = geometry_shader->get(definitions);
            s->bind();

            s->set("mvp", mvp);
            s->set("m", m);
            s->set("n_m", n_m);

            group.mat->apply(s);
            group.mesh->draw();
        }
    }
}
