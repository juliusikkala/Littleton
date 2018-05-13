#include "ssao.hh"
#include "shader.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include "resource_pool.hh"
#include "camera.hh"
#include "scene.hh"
#include "common_resources.hh"
#include "math.hh"

namespace lt::method
{

static texture* generate_ssao_kernel(
    context& ctx,
    unsigned size
){
    std::vector<glm::vec3> samples;
    samples.resize(size);

    for(unsigned i = 0; i < size; ++i)
    {
        glm::vec3 dir = glm::sphericalRand(1.0f);
        dir.z = fabs(dir.z);
        float scale = (float)i / size;
        samples[i] = dir * glm::mix(0.1f, 1.0f, scale * scale);
    }

    return new texture(
        ctx,
        glm::uvec2(size),
        GL_RGB8_SNORM,
        GL_FLOAT,
        0,
        GL_TEXTURE_1D,
        (float*)samples.data()
    );
}

ssao::ssao(
    render_target& target,
    gbuffer& buf,
    resource_pool& pool,
    render_scene* scene,
    float radius, 
    unsigned samples,
    unsigned blur_radius,
    float bias
):  target_method(target), glresource(pool.get_context()), buf(&buf),
    ssao_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "ssao.frag"}, {}
    )),
    vertical_blur_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "blur.frag"}, {{"VERTICAL", ""}}
    )),
    horizontal_blur_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "blur.frag"}, {{"HORIZONTAL", ""}}
    )),
    ambient_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "ambient.frag"}, {}
    )),
    scene(scene),
    ssao_buffer(get_context(), target.get_size(), GL_R8),
    radius(radius), samples(samples), blur_radius(blur_radius), bias(bias),
    random_rotation(
        common::ensure_spherical_random_texture(pool, glm::uvec2(4))
    ),
    kernel(generate_ssao_kernel(get_context(), samples)),
    quad(common::ensure_quad_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool)),
    noise_sampler(get_context(), GL_NEAREST, GL_NEAREST, GL_REPEAT, 0)
{
}

void ssao::set_radius(float radius)
{
    this->radius = radius;
}

float ssao::get_radius() const
{
    return radius;
}

void ssao::set_samples(unsigned samples)
{
    kernel.reset(generate_ssao_kernel(get_context(), samples)),
    this->samples = samples;
}

unsigned ssao::get_samples() const
{
    return samples;
}

void ssao::set_blur(unsigned blur_radius)
{
    this->blur_radius = blur_radius;
}

unsigned ssao::get_blur() const
{
    return blur_radius;
}

void ssao::set_bias(float bias)
{
    this->bias = bias;
}

float ssao::get_bias() const
{
    return bias;
}

void ssao::execute()
{
    if(!ssao_shader || radius <= 0.0f || !scene ||
        scene->get_ambient() == glm::vec3(0))
        return;
    camera* cam = scene->get_camera();
    if(!cam) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDisable(GL_STENCIL_TEST);

    glm::mat4 p = cam->get_projection();

    ssao_buffer.input().bind();

    ssao_shader->bind();
    ssao_shader->set("in_depth", fb_sampler.bind(*buf->get_depth_stencil(), 0));
    ssao_shader->set("in_normal", fb_sampler.bind(*buf->get_normal(), 1));
    ssao_shader->set("radius", radius);
    ssao_shader->set<int>("samples", samples);
    ssao_shader->set("bias", bias);
    ssao_shader->set("proj", p);
    ssao_shader->set("projection_info", cam->get_projection_info());
    ssao_shader->set("clip_info", cam->get_clip_info());
    ssao_shader->set("noise", noise_sampler.bind(random_rotation, 2));
    ssao_shader->set("kernel", noise_sampler.bind(*kernel, 3));

    quad.draw();

    // Blur if necessary
    if(blur_radius != 0)
    {
        ssao_buffer.swap();
        ssao_buffer.input().bind();

        vertical_blur_shader->set(
            "tex", fb_sampler.bind(ssao_buffer.output())
        );
        vertical_blur_shader->set("samples", (int)(2 * blur_radius + 1));
        quad.draw();

        ssao_buffer.swap();
        ssao_buffer.input().bind();

        horizontal_blur_shader->set(
            "tex", fb_sampler.bind(ssao_buffer.output())
        );
        horizontal_blur_shader->set("samples", (int)(2 * blur_radius + 1));
        quad.draw();
    }

    glEnable(GL_BLEND);
    ssao_buffer.swap();
    get_target().bind();

    ambient_shader->bind();
    ambient_shader->set("in_color", fb_sampler.bind(*buf->get_color(), 0));

    ambient_shader->set("ambient", scene->get_ambient());
    ambient_shader->set("occlusion", fb_sampler.bind(ssao_buffer.output(), 2));

    quad.draw();
}

std::string ssao::get_name() const
{
    return "ssao";
}

} // namespace lt::method
