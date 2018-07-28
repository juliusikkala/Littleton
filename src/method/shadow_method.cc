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
#include "shadow_method.hh"

namespace lt::method
{

shadow_method::shadow_method(Scene scene)
: scene_method(scene)
{
}

void shadow_method::set_directional_uniforms(shader*, unsigned&) {}
void shadow_method::set_omni_uniforms(shader*, unsigned&) {}
void shadow_method::set_perspective_uniforms(shader*, unsigned&) {}

shader::definition_map
method::shadow_method::get_directional_definitions() const
{
    return {};
}

shader::definition_map shadow_method::get_omni_definitions() const
{
    return {};
}

shader::definition_map
method::shadow_method::get_perspective_definitions() const
{
    return {};
}

void shadow_method::set_shadow_map_uniforms(
    shader*,
    unsigned&,
    directional_shadow_map*,
    const std::string&,
    const glm::mat4& 
){}

void shadow_method::set_shadow_map_uniforms(
    shader*,
    unsigned&,
    omni_shadow_map*,
    const std::string&,
    const glm::mat4&
){}

void shadow_method::set_shadow_map_uniforms(
    shader*,
    unsigned&,
    perspective_shadow_map*,
    const std::string&,
    const glm::mat4&
){}

} // namespace lt::method
