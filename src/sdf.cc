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
#include <boost/algorithm/string.hpp>
#include <sstream>

namespace lt
{

sdf_object::sdf_object(
    const material* mat,
    const std::string& distance_func,
    const std::string& texture_mapping_func
):  mat(mat), distance_func(distance_func)
{
    set_material(mat, texture_mapping_func);
}

sdf_object::sdf_object(
    const std::string& distance_func,
    const std::string& material_func
):  mat(nullptr), distance_func(distance_func), material_func(material_func)
{
    recalc_hash();
}

void sdf_object::set_distance_func(const std::string& distance_func)
{
    this->distance_func = distance_func;
    recalc_hash();
}

const std::string& sdf_object::get_distance_func() const
{
    return distance_func;
}

void sdf_object::set_material(
    const material* mat,
    const std::string& texture_mapping_func
)
{
    this->mat = mat;
    // TODO: Create material_func from texture_mapping_func
    recalc_hash();
}

void sdf_object::set_material(const std::string& material_func)
{
    this->material_func = material_func;
    recalc_hash();
}

uint64_t sdf_object::get_hash() const
{
    return hash;
}

void sdf_object::recalc_hash()
{
    uint64_t seed = 0;
    boost::hash_combine(seed, distance_func);
    boost::hash_combine(seed, material_func);
    hash = seed;
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

void sdf_scene::update_definitions(shader::definition_map& def)
{
    // Generate code here
    std::stringstream distance_func_src;
    std::stringstream map_src;

    map_src
        << "float map(in vec3 p) {"
        << "float closest = INF;";

    for(unsigned i = 0; i < objects.size(); ++i)
    {
        distance_func_src
           << "float distance_sdf_object_" << i << "(in vec3 p) {"
           << objects[i]->get_distance_func()
           << "} ";

        map_src
            << "closest = min(closest, distance_sdf_object_" << i << "(p));";
    }
    map_src
        << "return closest;"
        << "}";

    std::string src = distance_func_src.str() + map_src.str();
    boost::replace_all(src, "\n", "\\\n");
    def["SDF_INSERT_CODE"] = src;
}

uint64_t sdf_scene::get_hash() const
{
    uint64_t seed = 0;
    for(sdf_object* obj: objects)
        boost::hash_combine(seed, obj->get_hash());
    return seed;
}

}
