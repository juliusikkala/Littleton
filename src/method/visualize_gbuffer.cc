#include "visualize_gbuffer.hh"
#include "multishader.hh"
#include "camera.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include "resource_pool.hh"
#include "common_resources.hh"
#include "scene.hh"

method::visualize_gbuffer::visualize_gbuffer(
    render_target& target,
    gbuffer& buf,
    resource_pool& pool,
    render_scene* scene
):  target_method(target), buf(&buf),
    visualize_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "visualize.frag"}
    )),
    scene(scene),
    quad(common::ensure_quad_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool)),
    visualizers({POSITION, NORMAL, COLOR, MATERIAL})
{
}

void method::visualize_gbuffer::set_scene(render_scene* scene)
{
    this->scene = scene;
}

render_scene* method::visualize_gbuffer::get_scene() const
{
    return scene;
}

void method::visualize_gbuffer::show(visualizer full)
{
    visualizers = {full};
}

void method::visualize_gbuffer::show(
    visualizer topleft,
    visualizer topright,
    visualizer bottomleft,
    visualizer bottomright
){
    visualizers = {topleft, topright, bottomleft, bottomright};
}

static void render_visualizer(
    const gbuffer* buf,
    method::visualize_gbuffer::visualizer v,
    multishader* ms,
    const primitive& quad,
    camera* cam
){
    shader* s;
    switch(v)
    {
    case method::visualize_gbuffer::DEPTH:
        s = ms->get({{"SHOW_DEPTH", ""}});
        break;
    case method::visualize_gbuffer::POSITION:
        s = ms->get({{"SHOW_POSITION", ""}});
        break;
    case method::visualize_gbuffer::NORMAL:
        s = ms->get({{"SHOW_NORMAL", ""}});
        break;
    case method::visualize_gbuffer::COLOR:
        s = ms->get({{"SHOW_COLOR", ""}});
        break;
    case method::visualize_gbuffer::ROUGHNESS:
        s = ms->get({{"SHOW_ROUGHNESS", ""}});
        break;
    case method::visualize_gbuffer::METALLIC:
        s = ms->get({{"SHOW_METALLIC", ""}});
        break;
    case method::visualize_gbuffer::IOR:
        s = ms->get({{"SHOW_IOR", ""}});
        break;
    case method::visualize_gbuffer::MATERIAL:
        s = ms->get({{"SHOW_MATERIAL", ""}});
        break;
    default:
        throw std::runtime_error("Unknown visualizer type");
    }
    s->bind();
    buf->set_uniforms(s);
    s->set("projection_info", cam->get_projection_info());
    s->set("clip_info", cam->get_clip_info());
    quad.draw();
}

void method::visualize_gbuffer::execute()
{
    target_method::execute();

    if(!visualize_shader || visualizers.size() == 0 || !scene) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    camera* cam = scene->get_camera();
    if(!cam) return;

    buf->bind_textures(fb_sampler);

    if(visualizers.size() == 1)
    {
        render_visualizer(buf, visualizers[0], visualize_shader, quad, cam);
    }
    else if(visualizers.size() == 4)
    {
        glm::uvec2 size = get_target().get_size();
        glm::uvec2 half_size = size/2u;

        glViewport(0, half_size.y, half_size.x, half_size.y);
        render_visualizer(buf, visualizers[0], visualize_shader, quad, cam);

        glViewport(half_size.x, half_size.y, half_size.x, half_size.y);
        render_visualizer(buf, visualizers[1], visualize_shader, quad, cam);

        glViewport(0, 0, half_size.x, half_size.y);
        render_visualizer(buf, visualizers[2], visualize_shader, quad, cam);

        glViewport(half_size.x, 0, half_size.x, half_size.y);
        render_visualizer(buf, visualizers[3], visualize_shader, quad, cam);

        glViewport(0, 0, size.x, size.y);
    }
}

std::string method::visualize_gbuffer::get_name() const
{
    return "visualize_gbuffer";
}
