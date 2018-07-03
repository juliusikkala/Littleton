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

stencil_handler::stencil_handler(
    GLenum func,
    unsigned ref,
    unsigned mask
): func(func), ref(ref), mask(mask) {}

void stencil_handler::set_stencil(
    GLenum func,
    unsigned ref,
    unsigned mask
){
    this->func = func;
    this->ref = ref;
    this->mask = mask;
}

void stencil_handler::set_stencil_ref(unsigned ref)
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
    glStencilFunc(GL_ALWAYS, ref, mask);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(mask);
}

void stencil_handler::stencil_cull()
{
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(func, ref, mask);
    glStencilMask(0x00);
}

void stencil_handler::stencil_draw_cull()
{
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(func, ref, mask);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(mask);
}

} // namespace lt
