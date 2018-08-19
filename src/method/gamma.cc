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
#include "gamma.hh"
#include "render_target.hh"
#include "texture.hh"
#include "sampler.hh"
#include "resource_pool.hh"
#include "common_resources.hh"

namespace lt::method
{

gamma::gamma(
    render_target& target,
    texture& src,
    resource_pool& pool,
    const options& opt
):  target_method(target), options_method(opt), src(&src),
    gamma_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "gamma.frag"}, {})
    ),
    quad(common::ensure_quad_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool))
{
}

void gamma::execute()
{
    target_method::execute();

    if(!gamma_shader || !src) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    gamma_shader->bind();
    gamma_shader->set("gamma", 1.0f/opt.gamma);
    gamma_shader->set("in_color", fb_sampler.bind(*src));

    quad.draw();
}

} // namespace lt::method
