#include "render_shadow_maps.hh"

method::render_shadow_maps::render_shadow_maps(
    shader_store& store,
    render_scene* scene
): depth_shader(
       store.get(
           shader::path{"generic.vert", "empty.frag"},
           {{"VERTEX_POSITION", "0"}}
       )
   ),
   scene(scene)
{}

void method::render_shadow_maps::set_scene(render_scene* s)
{
    scene = s;
}

render_scene* method::render_shadow_maps::get_scene() const
{
    return scene;
}

void method::render_shadow_maps::execute()
{
    if(!depth_shader || !scene) return;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    depth_shader->bind();

    for(
        directional_shadow_map* shadow_map:
        scene->get_directional_shadow_maps()
    ){
        shadow_map->bind();
        glClear(GL_DEPTH_BUFFER_BIT);

        glm::mat4 v = shadow_map->get_view();
        glm::mat4 p = shadow_map->get_projection();
        glm::mat4 vp = p * v;

        for(object* obj: scene->get_objects())
        {
            model* mod = obj->get_model();
            if(!mod) continue;

            glm::mat4 mvp = vp * obj->get_global_transform();

            for(model::vertex_group& group: *mod)
            {
                if(!group.mesh) continue;

                depth_shader->set("mvp", mvp);
                group.mesh->draw();
            }
        }
    }
}
