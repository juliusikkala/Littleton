#include "visualize_gbuffer.hh"
#include "multishader.hh"
#include "camera.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include "shader_pool.hh"
#include "scene.hh"

method::visualize_gbuffer::visualize_gbuffer(
    render_target& target,
    gbuffer& buf,
    shader_pool& store,
    render_scene* scene
):  target_method(target), buf(&buf),
    visualize_shader(store.get(
        shader::path{"fullscreen.vert", "visualize.frag"}
    )),
    scene(scene),
    fullscreen_quad(vertex_buffer::create_square(target.get_context())),
    gbuf_sampler(
        target.get_context(),
        GL_NEAREST,
        GL_NEAREST,
        GL_CLAMP_TO_EDGE
    ),
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
    method::visualize_gbuffer::visualizer v,
    multishader* ms,
    vertex_buffer& quad,
    glm::vec4 perspective_data
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
    s->set("in_depth", 0);
    s->set("in_color_emission", 1);
    s->set("in_normal", 2);
    s->set("in_material", 3);
    s->set("perspective_data", perspective_data);
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

    float near, far, fov, aspect;
    decompose_perspective(cam->get_projection(), near, far, fov, aspect);
    glm::vec4 perspective_data = glm::vec4(
        2*tan(fov/2.0f)*aspect,
        2*tan(fov/2.0f),
        near,
        far
    );

    gbuf_sampler.bind(buf->get_depth_stencil().bind(0));
    gbuf_sampler.bind(buf->get_color_emission().bind(1));
    gbuf_sampler.bind(buf->get_normal().bind(2));
    gbuf_sampler.bind(buf->get_material().bind(3));

    if(visualizers.size() == 1)
    {
        render_visualizer(
            visualizers[0],
            visualize_shader,
            fullscreen_quad,
            perspective_data
        );
    }
    else if(visualizers.size() == 4)
    {
        glm::uvec2 size = get_target().get_size();
        glm::uvec2 half_size = size/2u;

        glViewport(0, half_size.y, half_size.x, half_size.y);
        render_visualizer(
            visualizers[0],
            visualize_shader,
            fullscreen_quad,
            perspective_data
        );
        glViewport(half_size.x, half_size.y, half_size.x, half_size.y);
        render_visualizer(
            visualizers[1],
            visualize_shader,
            fullscreen_quad,
            perspective_data
        );
        glViewport(0, 0, half_size.x, half_size.y);
        render_visualizer(
            visualizers[2],
            visualize_shader,
            fullscreen_quad,
            perspective_data
        );
        glViewport(half_size.x, 0, half_size.x, half_size.y);
        render_visualizer(
            visualizers[3],
            visualize_shader,
            fullscreen_quad,
            perspective_data
        );
        glViewport(0, 0, size.x, size.y);
    }
}

std::string method::visualize_gbuffer::get_name() const
{
    return "visualize_gbuffer";
}
