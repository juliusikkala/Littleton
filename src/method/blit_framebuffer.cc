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
#include "blit_framebuffer.hh"

namespace lt::method
{

blit_framebuffer::blit_framebuffer(
    render_target& dst,
    render_target& src,
    blit_type type
): target_method(dst), src(&src), type(type) {}

void blit_framebuffer::set_blit_type(blit_type type)
{
    this->type = type;    
}

void blit_framebuffer::set_src(render_target& src)
{
    this->src = &src;
}

void blit_framebuffer::execute()
{
    target_method::execute();

    render_target& dst = get_target();

    src->bind(GL_READ_FRAMEBUFFER);
    dst.bind(GL_DRAW_FRAMEBUFFER);

    glm::uvec2 src_size = src->get_size();
    glm::uvec2 dst_size = dst.get_size();

    glBlitFramebuffer(
        0, 0, src_size.x, src_size.y,
        0, 0, dst_size.x, dst_size.y,
        type,
        type == COLOR_ONLY ? GL_LINEAR : GL_NEAREST
    );
}

std::string blit_framebuffer::get_name() const
{
    return "blit_framebuffer";
}

} // namespace lt::method
