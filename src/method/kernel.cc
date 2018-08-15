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
#include "kernel.hh"
#include "render_target.hh"
#include "texture.hh"
#include "sampler.hh"
#include "resource_pool.hh"
#include "common_resources.hh"

namespace lt::method
{

const glm::mat3 kernel::options::SHARPEN = glm::mat3(
     0, -1, 0,
    -1, 5, -1,
     0, -1, 0
);

const glm::mat3 kernel::options::EDGE_DETECT = glm::mat3(
    -1, -1, -1,
    -1, 8, -1,
    -1, -1, -1
)/16.0f;

const glm::mat3 kernel::options::GAUSSIAN_BLUR = glm::mat3(
    1, 2, 1,
    2, 4, 2,
    1, 2, 1
)/16.0f;

const glm::mat3 kernel::options::BOX_BLUR = glm::mat3(
    1, 1, 1,
    1, 1, 1,
    1, 1, 1
)/9.0f;

kernel::kernel(
    render_target& target,
    texture& src,
    resource_pool& pool,
    const options& opt
):  target_method(target), options_method(opt), src(&src),
    kernel_shader(
        pool.get_shader(shader::path{"fullscreen.vert", "kernel.frag"}, {})
    ),
    quad(common::ensure_quad_primitive(pool)),
    fb_sampler(common::ensure_framebuffer_sampler(pool))
{
}

void kernel::execute()
{
    target_method::execute();

    if(!kernel_shader || !src) return;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    kernel_shader->bind();
    kernel_shader->set("kernel", opt.kernel);
    kernel_shader->set("in_color", fb_sampler.bind(*src));

    quad.draw();
}

std::string kernel::get_name() const
{
    return "kernel";
}

} // namespace lt::method
