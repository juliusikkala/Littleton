/*
    Copyright 2018 Julius Ikkala

    This file is part of Littleton.

    Littleton is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Littleton is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Littleton.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "ssao.hh"
#include "multishader.hh"
#include "shader.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include "resource_pool.hh"
#include "camera.hh"
#include "scene.hh"
#include "common_resources.hh"
#include "math.hh"

namespace
{
using namespace lt;

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

}

namespace lt::method
{

ssao::ssao(
    render_target& target,
    gbuffer& buf,
    resource_pool& pool,
    Scene scene,
    const options& opt
):  target_method(target),
    scene_method(scene),
    options_method(opt),
    glresource(pool.get_context()),
    buf(&buf),
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
        shader::path{"fullscreen.vert", "ambient.frag"}
    )),
    ssao_buffer(get_context(), target.get_size(), GL_R8),
    random_rotation(
        common::ensure_spherical_random_texture(pool, glm::uvec2(4))
    ),
    quad(common::ensure_quad_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool)),
    noise_sampler(get_context(), GL_NEAREST, GL_NEAREST, GL_REPEAT, 0)
{
    options_will_update(opt, true);
}

void ssao::execute()
{
    const auto [radius, samples, blur_radius, bias] = opt;

    if(!has_all_scenes() || radius <= 0.0f)
        return;

    glm::vec3 ambient = get_scene<light_scene>()->get_ambient();

    if(ambient == glm::vec3(0))
        return;

    camera* cam = get_scene<camera_scene>()->get_camera();
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

    texture* indirect = buf->get_indirect_lighting();
    shader::definition_map ambient_definitions;
    if(indirect) ambient_definitions["USE_INDIRECT_LIGHTING"];
    shader* a = ambient_shader->get(ambient_definitions);

    a->bind();

    if(indirect) a->set("in_indirect_lighting", fb_sampler.bind(*indirect, 0));
    else a->set("in_color", fb_sampler.bind(*buf->get_color(), 0));

    a->set("ambient", ambient);
    a->set("occlusion", fb_sampler.bind(ssao_buffer.output(), 1));

    quad.draw();
}

void ssao::options_will_update(const options& next, bool initial)
{
    if(opt.samples != next.samples || initial)
        kernel.reset(generate_ssao_kernel(get_context(), next.samples));
}

} // namespace lt::method
