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
#ifndef LT_METHOD_GEOMETRY_PASS_HH
#define LT_METHOD_GEOMETRY_PASS_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../stencil_handler.hh"

namespace lt
{

class gbuffer;
class sampler;
class render_scene;
class multishader;
class shader;
class primitive;
class resource_pool;

}

namespace lt::method
{

class LT_API geometry_pass: public target_method, public stencil_handler
{
public:
    geometry_pass(
        gbuffer& buf,
        resource_pool& store,
        render_scene* scene,
        bool apply_ambient = true
    );

    void set_scene(render_scene* scene);
    render_scene* get_scene() const;

    void set_apply_ambient(bool apply_ambient);
    bool get_apply_ambient() const;

    void execute() override;

    std::string get_name() const override;

private:
    multishader* geometry_shader;
    shader* min_max_shader;
    render_scene* scene;
    const primitive& quad;
    const sampler& fb_sampler;

    bool apply_ambient;
};

} // namespace lt::method

#endif
