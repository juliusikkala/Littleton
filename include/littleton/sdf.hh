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
#ifndef LT_SDF_HH
#define LT_SDF_HH
#include "api.hh"
#include "transformable.hh"
#include <string>

namespace lt
{

class material;

/* 'distance_func' must be the contents of a glsl function with the following
 * definition:
 *
 * float distance_func(in vec3 p)
 * {
 *     <your code> // Your code must return a float
 * }
 *
 * If texture_mapping is defined, the textures of the given material are
 * used. 'texture_mapping_func' must be the contents of a glsl function with the
 * following definition:
 *
 * void texture_mapping_func(
 *     in vec3 p,
 *     in vec3 pdx,
 *     in vec3 pdy,
 *     out vec2 uv,
 *     out vec2 uvdx,
 *     out vec2 uvdy
 * ) {
 *     <your code>
 * }
 */
class LT_API sdf_object: public transformable_node
{
public:
    sdf_object(
        const material* mat,
        const std::string& distance_func,
        const std::string& texture_mapping_func = ""
    );

    void set_material(const material* mat);
    const material* get_material() const;

    void set_distance_func(const std::string& distance_func);
    const std::string& get_distance_func() const;

    void set_texture_mapping_func(const std::string& texture_mapping_func);
    const std::string& get_texture_mapping_func() const;

private:
    const material* mat;
    std::string distance_func;
    std::string texture_mapping_func;
};

class LT_API sdf_scene
{
public:
    sdf_scene();

    void add_sdf_object(sdf_object* object);
    void remove_sdf_object(sdf_object* object);
    const std::vector<sdf_object*>& get_sdf_objects() const;
    void clear_sdf_objects();

private:
    std::vector<sdf_object*> objects;
};

}

#endif

