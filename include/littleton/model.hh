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
#ifndef LT_MODEL_HH
#define LT_MODEL_HH
#include "api.hh"
#include <vector>
#include <cstddef>

namespace lt
{

class material;
class primitive;
class LT_API model
{
public:
    struct vertex_group
    {
        const material* mat;
        const primitive* mesh;
    };

    size_t group_count() const;
    void add_vertex_group(
        const material* mat,
        const primitive* mesh
    );
    void remove_vertex_group(size_t i);
    const vertex_group& operator[](size_t i) const;

    using iterator = std::vector<vertex_group>::iterator;
    using const_iterator = std::vector<vertex_group>::const_iterator;

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;

    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

private:
    std::vector<vertex_group> groups;
}; 

} // namespace lt

#endif
