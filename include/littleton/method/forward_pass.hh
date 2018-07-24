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
class shader;
class gbuffer;
class sampler;
class primitive;

}

namespace lt::method
{

class shadow_method;
class LT_API forward_pass:
    public target_method,
    public scene_method<camera_scene, object_scene, light_scene, shadow_scene>,
    public stencil_handler
{
public:
    forward_pass(
        render_target& target,
        resource_pool& pool,
        Scene scene,
        bool apply_ambient = true,
        bool apply_transmittance = true
    );

    forward_pass(
        gbuffer& target,
        resource_pool& pool,
        Scene scene,
        bool apply_ambient = true,
        bool apply_transmittance = true
    );

    ~forward_pass();

    // Whether to apply ambient lighting or not. Disable if you are using
    // SSAO or other methods which add the ambient term on their own.
    void set_apply_ambient(bool apply_ambient);
    bool get_apply_ambient() const;

    // Whether to apply object transmittance or not. Disable if you are
    // using SSRT or similar methods for refractions.
    void set_apply_transmittance(bool apply_transmittance);
    bool get_apply_transmittance() const;

    void render_opaque(bool opaque);
    void render_transparent(bool transparent);

    void execute() override;

    std::string get_name() const override;

private:
    multishader* forward_shader;
    multishader* cubemap_forward_shader;
    shader* min_max_shader;

    gbuffer* gbuf;

    bool opaque;
    bool transparent;

    bool apply_ambient;
    bool apply_transmittance;
    const primitive& quad;
    const sampler& fb_sampler;
};

} // namespace lt::method

#endif
