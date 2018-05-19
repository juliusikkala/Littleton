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
#ifndef LT_METHOD_FULLSCREEN_EFFECT_HH
#define LT_METHOD_FULLSCREEN_EFFECT_HH
#include "../pipeline.hh"
#include "../primitive.hh"
#include <map>
#include <string>

namespace lt
{

class texture;
class shader;
class resource_pool;

} // namespace lt

namespace lt::method
{

// Assumes the vertex shader is data/shaders/fullscreen.vert
class fullscreen_effect: public target_method
{
public:
    fullscreen_effect(
        render_target& target,
        resource_pool& pool,
        shader* effect = nullptr
    );
    ~fullscreen_effect();

    void execute() override;

    void set_shader(shader* effect);
    shader* get_shader() const;

    std::string get_name() const override;

private:
    shader* effect;
    const primitive& quad;
};

} // namespace lt::method

#endif
