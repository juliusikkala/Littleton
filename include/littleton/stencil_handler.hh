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
#ifndef LT_STENCIL_HANDLER_HH
#define LT_STENCIL_HANDLER_HH
#include "api.hh"
#include "glheaders.hh"

namespace lt
{

// To be used by methods for providing an interface for managing what is
// written to the stencil buffer and what is passed.
class LT_API stencil_handler
{
public:
    stencil_handler(
        GLenum func = GL_EQUAL,
        unsigned ref = 1,
        unsigned mask = 0xFF
    );

    void set_stencil(
        GLenum func = GL_EQUAL,
        unsigned ref = 1,
        unsigned mask = 0xFF
    );
    void set_stencil_ref(unsigned ref);

    // These should only be used by the owner of the handler, such as a method
    // deriving from it or one that holds it.
    void stencil_disable();
    void stencil_draw();
    void stencil_cull();
    void stencil_draw_cull();

private:
    GLenum func;
    unsigned ref;
    unsigned mask;
};

} // namespace lt

#endif
