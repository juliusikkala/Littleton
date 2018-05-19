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
#ifndef LT_POINT_LIGHT_HH
#define LT_POINT_LIGHT_HH
#include "../api.hh"
#include "transformable.hh"

namespace lt
{

class LT_API light
{
public:
    light(glm::vec3 color = glm::vec3(1.0));

    void set_color(glm::vec3 color);
    glm::vec3 get_color() const;

private:
    glm::vec3 color;
};

class LT_API directional_light: public light
{
public:
    directional_light(
        glm::vec3 color = glm::vec3(1.0),
        glm::vec3 direction = glm::vec3(0,-1,0)
    );

    void set_direction(glm::vec3 color);
    glm::vec3 get_direction() const;

private:
    glm::vec3 direction;
};

class LT_API point_light: public light, public transformable_node
{
public:
    point_light(glm::vec3 color = glm::vec3(1.0));
};

class LT_API spotlight: public point_light
{
public:
    spotlight(
        glm::vec3 color = glm::vec3(1.0),
        float cutoff_angle = 30,
        float falloff_exponent = 1
    );

    void set_cutoff_angle(float cutoff_angle);
    float get_cutoff_angle() const;

    void set_falloff_exponent(float falloff_exponent);
    float get_falloff_exponent() const;

    glm::vec3 get_global_direction() const;

private:
    float cutoff_angle;
    float falloff_exponent;
};

} // namespace lt

#endif
