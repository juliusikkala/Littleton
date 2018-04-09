#include "ssrt.hh"
#include "shader.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include "resource_pool.hh"
#include "camera.hh"
#include "scene.hh"
#include "common_resources.hh"
#include <glm/gtc/random.hpp>
#include <cmath>

static unsigned max_mipmap_index(glm::uvec2 size)
{
    return floor(log2(std::max(size.x, size.y)));
}

method::ssrt::ssrt(
    render_target& target,
    gbuffer& buf,
    resource_pool& pool,
    render_scene* scene
):  target_method(target), buf(&buf), pool(pool),
    ssrt_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "ssrt.frag"},
        {{"RAY_MAX_LEVEL", std::to_string(max_mipmap_index(target.get_size()))}}
    )),
    blit_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "blit_texture.frag"}, {}
    )),
    scene(scene),
    quad(common::ensure_quad_vertex_buffer(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool))
{
}

void method::ssrt::execute()
{
    if(!ssrt_shader || !scene || !buf) return;

    camera* cam = scene->get_camera();
    if(!cam) return;

    texture* linear_depth = buf->get_linear_depth();
    texture* lighting = buf->get_lighting();
    texture* normal = buf->get_normal();
    texture* material = buf->get_material();
    if(!linear_depth || !lighting || !normal || !material) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_STENCIL_TEST);

    glm::mat4 p = cam->get_projection();
    glm::uvec2 size(get_target().get_size());

    framebuffer_pool::loaner ssrt_buffer(pool.loan_framebuffer(
        size, {{GL_COLOR_ATTACHMENT0, {lighting->get_internal_format(), true}}}
    ));
    ssrt_buffer->bind();

    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);

    ssrt_shader->bind();
    ssrt_shader->set("in_linear_depth", fb_sampler.bind(*linear_depth, 0));
    ssrt_shader->set("in_lighting", fb_sampler.bind(*lighting, 1));
    ssrt_shader->set("in_normal", fb_sampler.bind(*normal, 2));
    ssrt_shader->set("in_material", fb_sampler.bind(*material, 3));

    ssrt_shader->set("proj", p);
    ssrt_shader->set("projection_info", cam->get_projection_info());
    ssrt_shader->set("clip_info", cam->get_clip_info());

    ssrt_shader->set("ray_max_steps", 1000);
    ssrt_shader->set("thickness", 0.5f);

    quad.draw();

    //glEnable(GL_BLEND);
    get_target().bind();

    blit_shader->bind();
    blit_shader->set(
        "tex",
        fb_sampler.bind(*ssrt_buffer->get_texture_target(GL_COLOR_ATTACHMENT0))
    );

    quad.draw();
}

std::string method::ssrt::get_name() const
{
    return "ssrt";
}
