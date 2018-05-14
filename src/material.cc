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
#include "material.hh"
#include "texture.hh"
#include "shader.hh"

namespace
{
using namespace lt;

void update_def(
    shader::definition_map& def,
    const material::sampler_tex& v,
    const char* key
){
    if(v.first == nullptr) def.erase(key);
    else def[key];
}

}

namespace lt
{

material::material()
:   color_factor(glm::vec4(1.0f)),
    color_texture(nullptr, nullptr),
    metallic_factor(0.0f),
    roughness_factor(1.0f),
    metallic_roughness_texture(nullptr, nullptr),
    normal_factor(1.0f),
    normal_texture(nullptr, nullptr),
    ior(1.45f),
    emission_factor(0.0f),
    emission_texture(nullptr, nullptr)
{}

void material::update_definitions(shader::definition_map& def) const
{
    update_def(def, color_texture, "MATERIAL_COLOR_TEXTURE");
    update_def(
        def,
        metallic_roughness_texture,
        "MATERIAL_METALLIC_ROUGHNESS_TEXTURE"
    );
    update_def(def, normal_texture, "MATERIAL_NORMAL_TEXTURE");
    update_def(def, emission_texture, "MATERIAL_EMISSION_TEXTURE");
}

void material::apply(shader* s, unsigned& texture_index) const
{
    s->set("material.color_factor", color_factor);
    if(color_texture.first) s->set(
        "material.color",
        color_texture.first->bind(*color_texture.second, texture_index++)
    );

    s->set("material.metallic_factor", metallic_factor);
    s->set("material.roughness_factor", roughness_factor);
    if(metallic_roughness_texture.first) s->set(
        "material.metallic_roughness",
        metallic_roughness_texture.first->bind(
            *metallic_roughness_texture.second,
            texture_index++
        )
    );

    s->set("material.normal_factor", normal_factor);
    if(normal_texture.first) s->set(
        "material.normal",
        normal_texture.first->bind(*normal_texture.second, texture_index++)
    );

    s->set<float>("material.f0", 2 * pow((ior-1)/(ior+1), 2));

    s->set("material.emission_factor", emission_factor);
    if(emission_texture.first) s->set(
        "material.emission",
        emission_texture.first->bind(*emission_texture.second, texture_index++)
    );
}

bool material::potentially_transparent() const
{
    return color_factor.a < 1.0f || (color_texture.second && 
        color_texture.second->get_external_format() == GL_RGBA);
}

} // namespace lt
