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
#ifndef LT_METHOD_FORWARD_PASS_HH
#define LT_METHOD_FORWARD_PASS_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../stencil_handler.hh"
#include "../scene.hh"

namespace lt
{

class resource_pool;
class multishader;
class gbuffer;

}

namespace lt::method
{

LT_OPTIONS(forward_pass)
{
    // Whether to apply ambient lighting or not. Disable if you are using
    // SSAO or other methods which add the ambient term on their own.
    bool apply_ambient = true;
    // Whether to apply object transmittance or not. Disable if you are
    // using SSRT or similar methods for refractions.
    bool apply_transmittance = true;
    // Whether to render opaque objects.
    bool render_opaque = true;
    // Whether to render transparent objects.
    bool render_transparent = true;
};

class shadow_method;
class LT_API forward_pass:
    public target_method,
    public scene_method<camera_scene, object_scene, light_scene, shadow_scene>,
    public options_method<forward_pass>,
    public stencil_handler
{
public:
    forward_pass(
        render_target& target,
        resource_pool& pool,
        Scene scene,
        const options& opt = {}
    );

    forward_pass(
        gbuffer& target,
        resource_pool& pool,
        Scene scene,
        const options& opt = {}
    );

    ~forward_pass();

    void execute() override;

private:
    multishader* forward_shader;
    multishader* cubemap_forward_shader;

    gbuffer* gbuf;
};

} // namespace lt::method

#endif
