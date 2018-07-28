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
#include "sdf.hh"
#include "helpers.hh"

namespace lt
{

sdf_object::sdf_object(
    const material* mat,
    const std::string& distance_func,
    const std::string& texture_mapping_func
):  mat(mat),
    distance_func(distance_func), texture_mapping_func(texture_mapping_func)
{}

void sdf_object::set_material(const material* mat)
{
    this->mat = mat;
}

const material* sdf_object::get_material() const
{
    return mat;
}

void sdf_object::set_distance_func(const std::string& distance_func)
{
    this->distance_func = distance_func;
}

const std::string& sdf_object::get_distance_func() const
{
    return distance_func;
}

void sdf_object::set_texture_mapping_func(
    const std::string& texture_mapping_func
){
    this->texture_mapping_func = texture_mapping_func;
}

const std::string& sdf_object::get_texture_mapping_func() const
{
    return texture_mapping_func;
}

sdf_scene::sdf_scene() {}

void sdf_scene::add_sdf_object(sdf_object* object)
{
    sorted_insert(objects, object);
}

void sdf_scene::remove_sdf_object(sdf_object* object)
{
    sorted_erase(objects, object);
}

const std::vector<sdf_object*>& sdf_scene::get_sdf_objects() const
{
    return objects;
}

void sdf_scene::clear_sdf_objects()
{
    objects.clear();
}

}
