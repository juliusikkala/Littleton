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
#include "clear_gbuffer.hh"
#include "gbuffer.hh"

namespace lt::method
{

clear_gbuffer::clear_gbuffer(gbuffer& gbuf): gbuf(&gbuf) {}
clear_gbuffer::~clear_gbuffer() {}

void clear_gbuffer::execute()
{
    gbuf->set_draw(gbuffer::DRAW_ALL);
    gbuf->clear();
    gbuf->set_draw(gbuffer::DRAW_LIGHTING);
}

} // namespace lt::method
