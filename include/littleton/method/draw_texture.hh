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
#ifndef LT_METHOD_DRAW_TEXTURE_HH
#define LT_METHOD_DRAW_TEXTURE_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../primitive.hh"
#include "../sampler.hh"

namespace lt
{

class texture;
class resource_pool;

}

namespace lt::method
{

// DEPRECATED. TODO: Remove when 2D rendering features are in place.
class LT_API draw_texture: public target_method
{
public:
    draw_texture(
        render_target& target,
        resource_pool& shaders,
        texture* tex = nullptr
    );
    ~draw_texture();

    void set_transform(glm::mat4 transform);

    void set_texture(texture* tex = nullptr);

    void execute() override;

    std::string get_name() const override;

private:
    const primitive& quad;
    sampler color_sampler;

    shader* draw_shader;
    glm::mat4 transform;
    texture* tex;
};

} // namespace lt::method

#endif

