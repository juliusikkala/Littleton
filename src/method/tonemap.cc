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
#include "tonemap.hh"
#include "render_target.hh"
#include "texture.hh"
#include "resource_pool.hh"
#include "common_resources.hh"

namespace lt::method
{

tonemap::tonemap(
    render_target& target,
    resource_pool& pool,
    texture* src,
    const options& opt
):  target_method(target), options_method(opt), src(src),
    tonemap_shader(
        pool.get_shader(shader::path{"fullscreen.vert", "tonemap.frag"}, {})
    ),
    quad(common::ensure_quad_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool))
{
}

void tonemap::execute()
{
    target_method::execute();

    if(!tonemap_shader || !src) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    tonemap_shader->bind();
    tonemap_shader->set<float>("exposure", opt.exposure);
    tonemap_shader->set("in_color", fb_sampler.bind(*src));

    quad.draw();
}

std::string tonemap::get_name() const
{
    return "tonemap";
}

} // namespace lt::method
