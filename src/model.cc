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
#include "model.hh"

namespace lt
{

size_t model::group_count() const
{
    return groups.size();
}

void model::add_vertex_group(
    const material* mat,
    const primitive* mesh
){
    groups.emplace_back(vertex_group{mat, mesh});
}

void model::remove_vertex_group(size_t i)
{
    groups.erase(groups.begin() + i);
}

const model::vertex_group& model::operator[](size_t i) const
{
    return groups[i];
}

model::iterator model::begin()
{
    return groups.begin();
}

model::const_iterator model::begin() const
{
    return groups.begin();
}

model::const_iterator model::cbegin() const
{
    return groups.cbegin();
}

model::iterator model::end()
{
    return groups.end();
}

model::const_iterator model::end() const
{
    return groups.end();
}

model::const_iterator model::cend() const
{
    return groups.cend();
}

} // namespace lt
