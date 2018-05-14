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
#include "bloom.hh"
#include "resource_pool.hh"
#include "shader.hh"
#include "multishader.hh"
#include "common_resources.hh"
#include "math.hh"

namespace lt::method
{

bloom::bloom(
    render_target& target,
    resource_pool& pool,
    texture* src,
    float threshold,
    unsigned radius,
    float strength,
    unsigned level
):  target_method(target), pool(pool), src(src),
    threshold_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "threshold.frag"}, {}
    )),
    convolution_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "convolution.frag"}
    )),
    apply_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "blend_texture.frag"}, {}
    )),
    threshold(threshold), radius(radius), strength(strength), level(level),
    quad(common::ensure_quad_primitive(pool)),
    smooth_sampler(pool.get_context(), GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE)
{
    set_radius(radius);
}

void bloom::set_threshold(float threshold)
{
    this->threshold = threshold;
}

float bloom::get_threshold() const
{
    return threshold;
}

void bloom::set_radius(unsigned radius)
{
    this->radius = radius;

    gaussian_kernel = generate_gaussian_kernel(
        (radius >> level), 0.4f * (radius >> level)
    );
}

unsigned bloom::get_radius() const
{
    return radius;
}

void bloom::set_strength(float strength)
{
    this->strength = strength;
}

float bloom::get_strength() const
{
    return strength;
}

void bloom::set_level(unsigned level)
{
    this->level = level;

    gaussian_kernel = generate_gaussian_kernel(
        (radius >> level), 0.4f * (radius >> level)
    );
}

unsigned bloom::get_level() const
{
    return level;
}

void bloom::execute()
{
    if(radius <= 0.0f || strength <= 0.0f || !src)
        return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    auto tmp1_fb = pool.loan_framebuffer(
        get_target().get_size() >> level,
        {{GL_COLOR_ATTACHMENT0, {src->get_internal_format(), true}}}
    );

    auto tmp2_fb = pool.loan_framebuffer(
        get_target().get_size() >> level,
        {{GL_COLOR_ATTACHMENT0, {src->get_internal_format(), true}}}
    );

    tmp1_fb->bind();

    threshold_shader->bind();

    threshold_shader->set("src_color", smooth_sampler.bind(*src));
    threshold_shader->set("threshold", threshold);
    threshold_shader->set("fail_color", glm::vec4(0,0,0,1));

    quad.draw();

    tmp2_fb->bind();

    shader* cs = convolution_shader->get(
        {{"MAX_CONVOLUTION_SIZE", std::to_string((radius >> level) * 2 + 1)}}
    );

    cs->bind();
    cs->set(
        "src",
        smooth_sampler.bind(*tmp1_fb->get_texture_target(GL_COLOR_ATTACHMENT0))
    );
    cs->set("kernel", gaussian_kernel.size(), gaussian_kernel.data());
    cs->set("convolution_size", (int)(2 * (radius >> level) + 1));
    cs->set("step_size", glm::ivec2(0,1));

    quad.draw();

    tmp1_fb->bind();

    cs->set(
        "src",
        smooth_sampler.bind(*tmp2_fb->get_texture_target(GL_COLOR_ATTACHMENT0))
    );
    cs->set("step_size", glm::ivec2(1,0));
    quad.draw();

    get_target().bind();

    apply_shader->bind();
    apply_shader->set("src1", smooth_sampler.bind(*src, 0));
    apply_shader->set(
        "src2",
        smooth_sampler.bind(
            *tmp1_fb->get_texture_target(GL_COLOR_ATTACHMENT0), 1
        )
    );
    apply_shader->set("multiplier1", 1.0f);
    apply_shader->set("multiplier2", strength);

    quad.draw();
}

std::string bloom::get_name() const
{
    return "bloom";
}

} // namespace lt::method
