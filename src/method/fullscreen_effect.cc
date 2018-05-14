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
#include "fullscreen_effect.hh"
#include "helpers.hh"
#include "render_target.hh"
#include "shader.hh"
#include "texture.hh"
#include "common_resources.hh"

namespace lt::method
{

fullscreen_effect::fullscreen_effect(
    render_target& target,
    resource_pool& pool,
    shader* effect
): target_method(target), effect(effect),
   quad(common::ensure_quad_primitive(pool))
{}

fullscreen_effect::~fullscreen_effect() { }

void fullscreen_effect::execute()
{
    target_method::execute();

    if(!effect) return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    effect->bind();

    quad.draw();
}

void fullscreen_effect::set_shader(shader* effect)
{
    this->effect = effect;
}

shader* fullscreen_effect::get_shader() const
{
    return effect;
}

std::string fullscreen_effect::get_name() const
{
    return "fullscreen_effect";
}

} // namespace lt::method
