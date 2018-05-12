#include "geometry_pass.hh"
#include "camera.hh"
#include "model.hh"
#include "object.hh"
#include "material.hh"
#include "primitive.hh"
#include "common_resources.hh"
#include "resource_pool.hh"
#include "multishader.hh"
#include "gbuffer.hh"
#include "shader_pool.hh"
#include "scene.hh"
#include <glm/glm.hpp>
#include <utility>
#include <glm/gtc/matrix_inverse.hpp>

method::geometry_pass::geometry_pass(
    gbuffer& buf,
    resource_pool& pool,
    render_scene* scene,
    bool apply_ambient
):  target_method(buf),
    geometry_shader(pool.get_shader(
        shader::path{"generic.vert", "forward.frag"})
    ),
    min_max_shader(buf.get_min_max_shader(pool)),
    scene(scene),
    quad(common::ensure_quad_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool)),
    apply_ambient(apply_ambient)
{}

void method::geometry_pass::set_scene(render_scene* scene)
{
    this->scene = scene;
}

render_scene* method::geometry_pass::get_scene() const
{
    return scene;
}

void method::geometry_pass::set_apply_ambient(bool apply_ambient)
{
    this->apply_ambient = apply_ambient;
}

bool method::geometry_pass::get_apply_ambient() const
{
    return apply_ambient;
}

void method::geometry_pass::execute()
{
    target_method::execute();
    if(!geometry_shader || !scene)
        return;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    stencil_draw();

    camera* cam = scene->get_camera();
    if(!cam) return;

    glm::mat4 v = glm::inverse(cam->get_global_transform());
    glm::mat4 p = cam->get_projection();

    shader::definition_map common({
        {"OUTPUT_GEOMETRY", ""},
        {"OUTPUT_LIGHTING", ""},
        {"APPLY_EMISSION", ""},
        {"MIN_ALPHA", "1.0f"}
    });

    if(apply_ambient) common["APPLY_AMBIENT"];

    gbuffer* gbuf = static_cast<gbuffer*>(&get_target());

    // Emission values are written to lighting during geometry pass, so DRAW_ALL
    // instead of DRAW_GEOMETRY.
    gbuf->set_draw(gbuffer::DRAW_ALL);
    gbuf->update_definitions(common);

    for(object* obj: scene->get_objects())
    {
        const model* mod = obj->get_model();
        if(!mod) continue;

        glm::mat4 mv = v * obj->get_global_transform();
        glm::mat3 n_m(glm::inverseTranspose(mv));
        glm::mat4 mvp = p * mv;

        for(const model::vertex_group& group: *mod)
        {
            if(!group.mat || !group.mesh) continue;

            shader::definition_map definitions(common);
            group.mat->update_definitions(definitions);
            group.mesh->update_definitions(definitions);

            shader* s = geometry_shader->get(definitions);
            s->bind();

            s->set("mvp", mvp);
            s->set("m", mv);
            s->set("n_m", n_m);
            s->set("ambient", scene->get_ambient());

            group.mat->apply(s);
            group.mesh->draw();
        }
    }

    gbuf->render_depth_mipmaps(min_max_shader, quad, fb_sampler);
    gbuf->set_draw(gbuffer::DRAW_LIGHTING);
}

std::string method::geometry_pass::get_name() const
{
    return "geometry_pass";
}
