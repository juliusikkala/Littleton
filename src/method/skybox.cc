#include "skybox.hh"
#include "common_resources.hh"
#include "scene.hh"
#include "vertex_buffer.hh"
#include "shader.hh"
#include "resource_pool.hh"
#include "environment_map.hh"
#include "camera.hh"

method::skybox::skybox(
    render_target& target,
    resource_pool& pool,
    render_scene* scene
):  target_method(target),
    sky_shader(pool.get_shader(
        shader::path{"skybox.vert", "skybox.frag"}, {}
    )),
    scene(scene),
    skybox_sampler(pool.get_context(), GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE),
    exposure(1.0f),
    quad(common::ensure_quad_vertex_buffer(pool))
{
    set_stencil_cull(0);
}

void method::skybox::set_scene(render_scene* s)
{
    this->scene = s;
}

render_scene* method::skybox::get_scene() const
{
    return scene;
}

void method::skybox::set_exposure(float exposure)
{
    this->exposure = exposure;
}

float method::skybox::get_exposure() const
{
    return exposure;
}

void method::skybox::execute()
{
    target_method::execute();

    if(!scene) return;
    environment_map* skybox = scene->get_skybox();
    camera* cam = scene->get_camera();
    if(!skybox || !cam) return;

    glm::mat4 p = cam->get_projection();

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    stencil_cull();

    sky_shader->bind();

    sky_shader->set("skybox", skybox_sampler.bind(*skybox));
    sky_shader->set("exposure", exposure);
    sky_shader->set("inv_view", cam->get_global_transform());
    sky_shader->set("projection", glm::inverse(p));

    quad.draw();
}

std::string method::skybox::get_name() const
{
    return "skybox";
}
