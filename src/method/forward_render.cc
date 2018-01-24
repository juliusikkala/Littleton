#include "forward_render.hh"
#include <glm/glm.hpp>

method::forward_render::forward_render(
    shader* forward_shader,
    scene* render_scene
): forward_shader(forward_shader), render_scene(render_scene) {}

method::forward_render::~forward_render() {}

void method::forward_render::execute()
{
    if(!forward_shader || !render_scene)
        return;


    camera* cam = render_scene->get_camera();
    if(!cam) return;

    forward_shader->bind();

    glm::mat4 vp = cam->get_projection() *
        glm::inverse(cam->get_global_transform());

    for(object* obj: *render_scene)
    {
        model* m = obj->get_model();
        if(!m) continue;

        glm::mat4 mvp = vp * obj->get_global_transform();
        forward_shader->set<glm::mat4>("mvp", mvp);

        for(model::vertex_group& group: *m)
        {
            if(!group.mat || !group.mesh) continue;

            group.mat->apply(forward_shader);
            group.mesh->draw();
        }
    }
}

void method::forward_render::set_shader(shader* s) { forward_shader = s;}
shader* method::forward_render::get_shader() const { return forward_shader; }

void method::forward_render::set_scene(scene* s) { render_scene = s; }
scene* method::forward_render::get_scene() const { return render_scene; }

shader* forward_shader;
scene* render_scene;
