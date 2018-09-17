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
#ifndef LT_GBUFFER_HH
#define LT_GBUFFER_HH
#include "api.hh"
#include "texture.hh"
#include "shader.hh"
#include "render_target.hh"
#include "math.hh"

namespace lt
{

class sampler;
class shader_pool;
class primitive;

class LT_API gbuffer: public render_target
{
public:
    // If depth_stencil isn't provided, an RBO is created for it.
    gbuffer(
        context& ctx,
        glm::uvec2 size,
        texture* normal = nullptr,
        texture* color = nullptr,
        texture* material = nullptr,
        texture* lighting = nullptr,
        texture* linear_depth = nullptr,
        texture* depth_stencil = nullptr,
        texture* indirect_lighting = nullptr
    );
    gbuffer(gbuffer&& other);
    ~gbuffer();

    texture* get_normal() const;
    texture* get_color() const;
    texture* get_material() const;
    texture* get_linear_depth() const;
    texture* get_lighting() const;
    texture* get_depth_stencil() const;
    texture* get_indirect_lighting() const;

    // -1 if not bound on drawBuffers
    int get_normal_index() const;
    int get_color_index() const;
    int get_material_index() const;
    int get_linear_depth_index() const;
    int get_lighting_index() const;
    int get_indirect_lighting_index() const;

    void bind_textures(const sampler& fb_sampler, unsigned& index) const;

    // Call bind_textures at least once before this!
    void set_uniforms(shader* s, unsigned& index) const;
    void update_definitions(shader::definition_map& def) const;

    // Assumes either draw_all or draw_geometry is in effect
    void clear();

    enum draw_mode
    {
        // Draws depth only
        DRAW_NONE = 0,
        DRAW_ALL,
        DRAW_GEOMETRY,
        // Default, since it works for almost everything that isn't written to
        // use a g-buffer.
        DRAW_LIGHTING,
        DRAW_INDIRECT_LIGHTING,
        DRAW_ALL_LIGHTING
    };

    void set_draw(draw_mode mode);
    draw_mode get_draw() const;

private:
    draw_mode mode;

    texture* normal;
    texture* color;
    texture* material;//roughness, metallic, ior
    texture* linear_depth;
    texture* lighting;
    texture* depth_stencil;
    GLuint depth_stencil_rbo;
    texture* indirect_lighting;

    int normal_index;
    int color_index;
    int material_index;
    int linear_depth_index;
    int lighting_index;
    int indirect_lighting_index;
};

} // namespace lt

#endif
