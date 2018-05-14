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
#include "draw_texture.hh"
#include "render_target.hh"
#include "texture.hh"
#include "resource_pool.hh"
#include "common_resources.hh"

namespace lt::method
{

draw_texture::draw_texture(
    render_target& target,
    resource_pool& pool,
    texture* tex
):  target_method(target),
    quad(common::ensure_quad_primitive(pool)),
    color_sampler(
        target.get_context(),
        GL_LINEAR,
        GL_LINEAR,
        GL_CLAMP_TO_EDGE
    ),
    transform(1.0f),
    tex(tex)
{
    shader::definition_map def;
    quad.update_definitions(def);

    draw_shader = pool.get_shader(
        shader::path{"generic.vert", "draw_texture.frag"},
        def
    );
}

draw_texture::~draw_texture()
{
}

void draw_texture::set_transform(glm::mat4 transform)
{
    this->transform = transform;
}

void draw_texture::set_texture(texture* tex)
{
    this->tex = tex;
}

void draw_texture::execute()
{
    target_method::execute();

    if(!draw_shader || !tex) return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    draw_shader->bind();

    draw_shader->set("mvp", transform);
    draw_shader->set("tex", color_sampler.bind(*tex, 0));

    quad.draw();
}

std::string draw_texture::get_name() const
{
    return "draw_texture";
}

} // namespace lt::method
