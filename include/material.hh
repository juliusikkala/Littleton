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
#ifndef LT_MATERIAL_HH
#define LT_MATERIAL_HH
#include "shader.hh"
#include "sampler.hh"
#include "math.hh"
#include <variant>

namespace lt
{

class texture;

class material
{
public:
    material();

    void update_definitions(shader::definition_map& def) const;
    void apply(shader* s, unsigned& texture_index) const;

    bool potentially_transparent() const;

    using sampler_tex = std::pair<const sampler*, const texture*>;

    glm::vec4 color_factor;
    sampler_tex color_texture;

    float metallic_factor;
    float roughness_factor;
    // metalness on B-channel, roughness on G-channel.
    sampler_tex metallic_roughness_texture;

    float normal_factor;
    sampler_tex normal_texture;

    float ior;

    glm::vec3 emission_factor;
    sampler_tex emission_texture;
};

} // namespace lt

#endif

