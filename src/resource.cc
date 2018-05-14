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
#include "resource.hh"

namespace lt
{

resource::resource(): loaded(false), do_unload(false), references(0) {}
resource::~resource() {}

void resource::load() const
{
    if(!loaded)
    {
        load_impl();
        loaded = true;
    }
    do_unload = false;
}

void resource::unload() const
{
    if(loaded)
    {
        if(references)
        {
            do_unload = true;
        }
        else
        {
            unload_impl();
            do_unload = false;
            loaded = false;
        }
    }
}

void resource::force_unload() const
{
    references = 0;
    unload();
}

void resource::load_impl() const { }

void resource::unload_impl() const { }

void resource::link() const
{
    references++;
}

void resource::unlink() const
{
    if(references) references--;
    if(do_unload && references == 0)
    {
        unload();
    }
}

glresource::glresource(context& ctx): ctx(&ctx) {}
context& glresource::get_context() const { return *ctx; }

} // namespace lt
