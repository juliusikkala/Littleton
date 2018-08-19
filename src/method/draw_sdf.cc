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
#include "draw_sdf.hh"
#include "multishader.hh"
#include "camera.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include "resource_pool.hh"
#include "common_resources.hh"
#include "scene.hh"
#include <stdexcept>

namespace lt::method
{

draw_sdf::draw_sdf(
    gbuffer& target,
    resource_pool& pool,
    Scene scene,
    const options& opt
):  target_method(target), scene_method(scene), options_method(opt),
    pool(pool),
    sdf_shaders(pool.get_shader(shader::path{"cast_ray.vert", "sdf.frag"})),
    sdf_shader(nullptr),
    quad(common::ensure_quad_primitive(pool)),
    linear_sampler(common::ensure_linear_sampler(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool)),
    mipmap_sampler(
        pool.get_context(),
        GL_NEAREST,
        GL_NEAREST_MIPMAP_NEAREST,
        GL_CLAMP_TO_EDGE
    )
{
    update_sdf_shader();
}

void draw_sdf::execute()
{
    target_method::execute();
    const auto [apply_ambient, insert] = opt;
    
    if(!sdf_shader || !has_all_scenes()) return;

    camera* cam = get_scene<camera_scene>()->get_camera();
    if(!cam) return;

    texture* linear_depth = gbuf->get_linear_depth();
    texture* lighting = gbuf->get_lighting();
    texture* normal = gbuf->get_normal();
    texture* material = gbuf->get_material();
    texture* color = gbuf->get_color();
    if(!linear_depth || !lighting || !normal || !material || !color)
        return;

    glm::mat4 p = cam->get_projection();
    glm::uvec2 size(get_target().get_size());

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gbuf->set_draw(gbuffer::DRAW_LIGHTING);

    stencil_draw();

    // TODO: Do we need indirect lighting here?
    // Copy lighting buffer for reading (the G-Buffer doesn't have double
    // buffering, TODO)
    framebuffer_pool::loaner sdf_buffer(pool.loan_framebuffer(
        size, {{GL_COLOR_ATTACHMENT0, {lighting->get_internal_format(), true}}}
    ));
    lighting = sdf_buffer->get_texture_target(GL_COLOR_ATTACHMENT0);

    sdf_buffer->bind(GL_DRAW_FRAMEBUFFER);
    gbuf->bind(GL_READ_FRAMEBUFFER);
    glBlitFramebuffer(
        0, 0, size.x, size.y,
        0, 0, size.x, size.y,
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
        GL_NEAREST
    );

    sdf_shader->bind();

    sdf_shader->set(
        "ivp",
        toMat4(cam->get_global_orientation()) * glm::inverse(p)
    );
    sdf_shader->set("in_linear_depth", mipmap_sampler.bind(*linear_depth, 0));
    sdf_shader->set("in_lighting", fb_sampler.bind(*lighting, 1));
    sdf_shader->set("in_normal", fb_sampler.bind(*normal, 2));
    sdf_shader->set("in_material", fb_sampler.bind(*material, 3));
    sdf_shader->set("in_color", fb_sampler.bind(*color, 4));

    sdf_shader->set("proj", p);
    sdf_shader->set("projection_info", cam->get_projection_info());
    sdf_shader->set("clip_info", cam->get_clip_info());
    sdf_shader->set("near", -cam->get_near());

    quad.draw();
}

void draw_sdf::options_will_update(const options& next)
{
    if(next.insert != opt.insert)
    {
        opt.insert = next.insert;
        update_sdf_shader();
    }
}

void draw_sdf::update_sdf_shader()
{
    shader::definition_map def(
        {{"INSERT", opt.insert}}
    );
    sdf_shader = sdf_shaders->get(def);
}

} // namespace lt::method
