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
#ifndef LT_METHOD_SKYBOX_HH
#define LT_METHOD_SKYBOX_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../sampler.hh"
#include "../stencil_handler.hh"

namespace lt
{

class shader;
class resource_pool;
class render_scene;
class primitive;

}

namespace lt::method
{

class LT_API skybox: public target_method, public stencil_handler
{
public:
    skybox(
        render_target& target,
        resource_pool& pool,
        render_scene* scene = nullptr
    );

    void set_scene(render_scene* s);
    render_scene* get_scene() const;

    void execute() override;

    std::string get_name() const override;

private:
    shader* sky_shader;
    render_scene* scene;
    sampler skybox_sampler;

    const primitive& quad;
};

} // namespace lt::method

#endif
