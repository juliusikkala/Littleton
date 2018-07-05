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
#include "sao.hh"
#include "multishader.hh"
#include "shader.hh"
#include "math.hh"
#include "gbuffer.hh"
#include "resource_pool.hh"
#include "camera.hh"
#include "scene.hh"
#include "common_resources.hh"

namespace lt::method
{

sao::sao(
    render_target& target,
    gbuffer& buf,
    resource_pool& pool,
    render_scene* scene,
    float radius,
    unsigned samples,
    float bias,
    float intensity
):  target_method(target), glresource(pool.get_context()), buf(&buf),
    ao_sample_pass_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "sao/ao_sample_pass.frag"},
        {{"USE_NORMAL_TEXTURE", ""}}
    )),
    blur_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "sao/blur.frag"}, {}
    )),
    ambient_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "ambient.frag"}
    )),
    scene(scene),
    radius(radius), samples(samples), bias(bias), intensity(intensity),
    ao(get_context(), target.get_size(), GL_R8),
    quad(common::ensure_quad_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool)),
    mipmap_sampler(
        get_context(),
        GL_NEAREST,
        GL_NEAREST_MIPMAP_NEAREST,
        GL_CLAMP_TO_EDGE
    )
{
    set_samples(samples);
}

void sao::set_scene(render_scene* scene)
{
    this->scene = scene;
}

render_scene* sao::get_scene() const
{
    return scene;
}

void sao::set_radius(float radius)
{
    this->radius = radius;
}

float sao::get_radius() const
{
    return radius;
}

void sao::set_samples(unsigned samples)
{
    this->samples = samples;

    spiral_turns = 17;
    for(unsigned i = samples + 1; i < 2048; ++i)
    {
        if(!factorize(i))
        {
            spiral_turns = (float)i;
            break;
        }
    }
}

unsigned sao::get_samples() const
{
    return samples;
}

void sao::set_bias(float bias)
{
    this->bias = bias;
}

float sao::get_bias() const
{
    return bias;
}

void sao::set_intensity(float intensity)
{
    this->intensity = intensity;
}

float sao::get_intensity() const
{
    return intensity;
}

void sao::execute()
{
    if(!scene || scene->get_ambient() == glm::vec3(0))
        return;

    camera* cam = scene->get_camera();
    texture* depth_tex = buf->get_linear_depth();
    if(!cam || !depth_tex) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDisable(GL_STENCIL_TEST);

    // Distributed AO sample pass
    ao.input().bind();

    glm::vec2 ppu = cam->pixels_per_unit(get_target().get_size());

    ao_sample_pass_shader->bind();
    ao_sample_pass_shader->set("in_depth", mipmap_sampler.bind(*depth_tex, 0));
    ao_sample_pass_shader->set(
        "in_normal",
        fb_sampler.bind(*buf->get_normal(), 1)
    );
    ao_sample_pass_shader->set("proj_scale", ppu.y);
    ao_sample_pass_shader->set("radius", radius);
    ao_sample_pass_shader->set<int>("samples", samples);
    ao_sample_pass_shader->set("bias", bias);
    ao_sample_pass_shader->set("spiral_turns", spiral_turns);
    ao_sample_pass_shader->set<float>(
        "multiplier",
        5.0f * intensity / (samples * pow(radius, 6))
    );
    ao_sample_pass_shader->set("projection_info", cam->get_projection_info());
    quad.draw();

    // Blur passes
    ao.swap();
    ao.input().bind();

    blur_shader->bind();
    blur_shader->set("in_ao", fb_sampler.bind(ao.output(), 0));
    blur_shader->set("in_depth", fb_sampler.bind(*depth_tex, 1));
    blur_shader->set("step_size", glm::ivec2(1, 0));

    quad.draw();

    ao.swap();
    ao.input().bind();

    blur_shader->set("in_ao", fb_sampler.bind(ao.output(), 0));
    blur_shader->set("step_size", glm::ivec2(0, 1));

    quad.draw();

    // Apply
    glEnable(GL_BLEND);
    ao.swap();
    get_target().bind();

    texture* indirect = buf->get_indirect_lighting();
    shader::definition_map ambient_definitions;
    if(indirect) ambient_definitions["USE_INDIRECT_LIGHTING"];
    shader* a = ambient_shader->get(ambient_definitions);

    a->bind();

    if(indirect) a->set("in_indirect_lighting", fb_sampler.bind(*indirect, 0));
    else a->set("in_color", fb_sampler.bind(*buf->get_color(), 0));

    a->set("ambient", scene->get_ambient());
    a->set("occlusion", fb_sampler.bind(ao.output(), 1));

    quad.draw();
}

std::string sao::get_name() const
{
    return "sao";
}

} // namespace lt::method
