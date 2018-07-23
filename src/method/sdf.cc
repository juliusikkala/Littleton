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
#include "sdf.hh"
#include "multishader.hh"
#include "camera.hh"
#include "helpers.hh"
#include "gbuffer.hh"
#include "resource_pool.hh"
#include "common_resources.hh"
#include "scene.hh"
#include <stdexcept>

namespace lt
{

sdf_object::sdf_object(
    const material* mat,
    const std::string& distance,
    const std::string& texture_mapping
): mat(mat), distance(distance), texture_mapping(texture_mapping) {}

void sdf_object::set_material(const material* mat)
{
    this->mat = mat;
}

const material* sdf_object::get_material() const
{
    return mat;
}

void sdf_object::set_distance(const std::string& distance)
{
    this->distance = distance;
}

const std::string& sdf_object::get_distance() const
{
    return distance;
}

void sdf_object::set_texture_mapping(const std::string& texture_mapping)
{
    this->texture_mapping = texture_mapping;
}

const std::string& sdf_object::get_texture_mapping() const
{
    return texture_mapping;
}

}

namespace lt::method
{

sdf::sdf(
    gbuffer& target,
    resource_pool& pool,
    render_scene* scene,
    const std::vector<sdf_object>& sdfs,
    bool apply_ambient,
    const std::string& insert
):  target_method(target),
    pool(pool),
    sdf_shaders(pool.get_shader(shader::path{"cast_ray.vert", "sdf.frag"})),
    sdf_shader(nullptr),
    scene(scene),
    apply_ambient(apply_ambient),
    quad(common::ensure_quad_primitive(pool)),
    linear_sampler(common::ensure_linear_sampler(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool)),
    mipmap_sampler(
        pool.get_context(),
        GL_NEAREST,
        GL_NEAREST_MIPMAP_NEAREST,
        GL_CLAMP_TO_EDGE
    ),
    insert(insert), sdfs(sdfs)
{
    update_sdf_shader();
}

void sdf::set_scene(render_scene* scene)
{
    this->scene = scene;
}

render_scene* sdf::get_scene() const
{
    return scene;
}

void sdf::set_sdfs(const std::vector<sdf_object>& sdfs)
{
    this->sdfs = sdfs;
    update_sdf_shader();
}

const std::vector<sdf_object>& sdf::get_sdfs() const
{
    return sdfs;
}

void sdf::set_apply_ambient(bool apply_ambient)
{
    this->apply_ambient = apply_ambient;
}

bool sdf::get_apply_ambient() const
{
    return apply_ambient;
}

shader* sdf::get_shader()
{
    return sdf_shader;
}

void sdf::execute()
{
    target_method::execute();
    
    if(!sdf_shader || !scene) return;

    camera* cam = scene->get_camera();
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

std::string sdf::get_name() const
{
    return "sdf";
}

void sdf::update_sdf_shader()
{
    shader::definition_map def(
        {{"INSERT", insert}}
    );
    sdf_shader = sdf_shaders->get(def);
}

} // namespace lt::method
