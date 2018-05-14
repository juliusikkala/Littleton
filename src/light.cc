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
#include "light.hh"

namespace lt
{

light::light(glm::vec3 color)
: color(color) {}

void light::set_color(glm::vec3 color)
{
    this->color = color;
}

glm::vec3 light::get_color() const
{
    return color;
}

directional_light::directional_light(glm::vec3 color, glm::vec3 direction)
: light(color), direction(direction) {}

void directional_light::set_direction(glm::vec3 direction)
{
    this->direction = direction;
}

glm::vec3 directional_light::get_direction() const
{
    return direction;
}

point_light::point_light(glm::vec3 color)
: light(color) {}

spotlight::spotlight(
    glm::vec3 color,
    float cutoff_angle,
    float falloff_exponent
):  point_light(color), cutoff_angle(cutoff_angle),
    falloff_exponent(falloff_exponent)
{
}

void spotlight::set_cutoff_angle(float cutoff_angle)
{
    this->cutoff_angle = cutoff_angle;
}

float spotlight::get_cutoff_angle() const
{
    return cutoff_angle;
}

void spotlight::set_falloff_exponent(float falloff_exponent)
{
    this->falloff_exponent = falloff_exponent;
}

float spotlight::get_falloff_exponent() const
{
    return falloff_exponent;
}

glm::vec3 spotlight::get_global_direction() const
{
    return get_global_orientation() * glm::vec3(0, 0, -1);
}

} // namespace lt
