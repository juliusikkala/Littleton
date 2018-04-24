#include "ssrt.hh"
#include "shader.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include "resource_pool.hh"
#include "camera.hh"
#include "scene.hh"
#include "multishader.hh"
#include "common_resources.hh"
#include "environment_map.hh"
#include <glm/gtc/random.hpp>
#include <cmath>
#include <stdexcept>

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
    ssrt_shaders(pool.get_shader(shader::path{"fullscreen.vert", "ssrt.frag"})),
    blit_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "blit_texture.frag"}, {}
    )),
    scene(scene),
    quad(common::ensure_quad_vertex_buffer(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool)),
    mipmap_sampler(
        pool.get_context(),
        GL_NEAREST,
        GL_NEAREST_MIPMAP_NEAREST,
        GL_CLAMP_TO_EDGE
    ),
    cubemap_sampler(pool.get_context(), GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE),
    max_steps(500),
    thickness(-1.0f),
    roughness_cutoff(0.5f),
    brdf_cutoff(0.0f),
    fallback_cubemap(false)
{
    set_thickness();
}

void method::ssrt::set_max_steps(unsigned max_steps)
{
    this->max_steps = max_steps;
}

void method::ssrt::set_roughness_cutoff(float cutoff)
{
    roughness_cutoff = cutoff;
}

void method::ssrt::set_brdf_cutoff(float cutoff)
{
    brdf_cutoff = cutoff;
}

void method::ssrt::set_thickness(float thickness)
{
    this->thickness = thickness;
    texture* linear_depth = buf->get_linear_depth();

    // Thickness requires min-max depth buffer
    if(thickness > 0.0f && linear_depth &&
       linear_depth->get_external_format() != GL_RG) 
    {
        throw std::runtime_error(
            "Min-max (two channel) linear depth buffer required for SSRT with "
            "finite thickness"
        );
    }

    refresh_shader();
}

void method::ssrt::use_fallback_cubemap(bool use)
{
    this->fallback_cubemap = use;
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
    texture* color_emission = buf->get_color_emission();
    if(!linear_depth || !lighting || !normal || !material || !color_emission)
        return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    stencil_cull();

    glm::mat4 p = cam->get_projection();
    glm::uvec2 size(get_target().get_size());

    framebuffer_pool::loaner ssrt_buffer(pool.loan_framebuffer(
        size, {{GL_COLOR_ATTACHMENT0, {lighting->get_internal_format(), true}}}
    ));
    ssrt_buffer->bind();

    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);

    ssrt_shader->bind();
    ssrt_shader->set("in_linear_depth", mipmap_sampler.bind(*linear_depth, 0));
    ssrt_shader->set("in_lighting", fb_sampler.bind(*lighting, 1));
    ssrt_shader->set("in_normal", fb_sampler.bind(*normal, 2));
    ssrt_shader->set("in_material", fb_sampler.bind(*material, 3));
    ssrt_shader->set("in_color_emission", fb_sampler.bind(*color_emission, 4));

    ssrt_shader->set("proj", p);
    ssrt_shader->set("projection_info", cam->get_projection_info());
    ssrt_shader->set("clip_info", cam->get_clip_info());
    ssrt_shader->set("near", -cam->get_near());

    ssrt_shader->set<int>("ray_max_steps", max_steps);
    ssrt_shader->set("thickness", thickness);
    ssrt_shader->set("roughness_cutoff", roughness_cutoff);
    ssrt_shader->set("brdf_cutoff", brdf_cutoff);

    environment_map* skybox = scene->get_skybox();
    if(fallback_cubemap && skybox)
    {
        ssrt_shader->set("fallback_cubemap", true);
        ssrt_shader->set("inv_view", cam->get_global_transform());
        ssrt_shader->set("env", cubemap_sampler.bind(*skybox, 5));
    }
    else
    {
        ssrt_shader->set("fallback_cubemap", false);
    }

    quad.draw();

    glEnable(GL_BLEND);
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

void method::ssrt::refresh_shader()
{
    shader::definition_map def = {
        {
            "RAY_MAX_LEVEL",
            std::to_string(max_mipmap_index(get_target().get_size()))
        }
    };
    if(thickness < 0.0f) def["INFINITE_THICKNESS"];
    if(fallback_cubemap) def["FALLBACK_CUBEMAP"];

    ssrt_shader = ssrt_shaders->get(def);
}
