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
#include "stencil_handler.hh"
#include "glheaders.hh"

namespace lt
{

stencil_handler::stencil_handler(): value(1), ref(1) {}

void stencil_handler::set_stencil_draw(unsigned value)
{
    this->value = value;
}

void stencil_handler::set_stencil_cull(unsigned ref)
{
    this->ref = ref;
}

void stencil_handler::stencil_disable()
{
    glDisable(GL_STENCIL_TEST);
}

void stencil_handler::stencil_draw()
{
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, value, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);
}

void stencil_handler::stencil_cull()
{
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, ref, 0xFF);
    glStencilMask(0x00);
}

} // namespace lt
