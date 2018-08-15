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
#include "clear.hh"
#include "glheaders.hh"

namespace lt::method
{

clear::clear(
    render_target& target,
    const options& opt
): target_method(target), options_method(opt) {}

clear::~clear(){}

void clear::execute()
{
    target_method::execute();
    const auto [color, depth, stencil] = opt;

    glStencilMask(0xFF);
    glClearColor(color.r, color.g, color.b, color.a);
    glClearDepth(depth);
    glClearStencil(stencil);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
}

std::string clear::get_name() const
{
    return "clear";
}

} // namespace lt::method
