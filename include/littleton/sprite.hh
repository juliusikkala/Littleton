/*
    Copyright 2019 Julius Ikkala

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
#ifndef LT_SPRITE_HH
#define LT_SPRITE_HH
#include "api.hh"
#include "transformable.hh"
#include "material.hh"
#include "animated.hh"
#include <vector>
#include <string>

namespace lt
{

class camera;
class texture;
class resource_pool;
class sprite_sheet;
class sampler;

// Handles sprite layout inside a sheet.
class LT_API sprite_layout
{
public:
    struct mode
    {
        // Leave these two empty if not a fake-3D sprite sheet. Add the only
        // rectangle in direction_frames alone. These are in degrees, [0, 360).
        // Yaw should be normalized to [0, 360), pitch to [-90, 90).
        // When looking up an angle, it is rounded to the nearest found step.
        std::vector<float> yaw_steps, pitch_steps;
        struct frame
        {
            // The duration this frame is active.
            duration frame_time;
            // The bounds should be normalized to [0, 1] uv coordinates.
            // Successive yaw steps are one after another, forming lines. After
            // each yaw step, the next pitch step is started.
            std::vector<vec4> bounds;
        };
        std::vector<frame> animation_frames;
    };

    explicit sprite_layout(const std::vector<mode>& layout);

    vec4 get_bounds(
        unsigned index = 0,
        duration animation_time = duration(0),
        float yaw_angle = 0.0f,
        float pitch_angle = 0.0f,
        bool looping = true
    ) const;

    // *_v are vertical configurations, successive modes are one after another.
    // *_h are the same thing but horizontal.
    static sprite_layout simple_v(unsigned modes = 1);
    static sprite_layout simple_h(unsigned modes = 1);
    static sprite_layout animated_v(
        unsigned frames,
        duration frame_time,
        unsigned modes = 1
    );
    static sprite_layout animated_h(
        unsigned frames,
        duration frame_time,
        unsigned modes = 1
    );
    static sprite_layout directional(
        unsigned yaw_steps, unsigned pitch_steps = 1
    );
    static sprite_layout directional(
        unsigned yaw_steps, const std::vector<float>& pitch_steps
    );
    static sprite_layout directional(
        const std::vector<float>& yaw_steps,
        const std::vector<float>& pitch_steps
    );

private:
    std::vector<mode> layout;
    std::vector<duration> animation_length;
};

class LT_API sprite: public transformable_node, public animated
{
public:
    sprite(
        resource_pool& pool,
        const std::string& texture_path,
        const sprite_layout* layout = nullptr,
        interpolation mag = interpolation::LINEAR,
        interpolation min = interpolation::LINEAR_MIPMAP_LINEAR
    );

    sprite(
        const texture* tex = nullptr,
        const sprite_layout* layout = nullptr,
        interpolation mag = interpolation::LINEAR,
        interpolation min = interpolation::LINEAR_MIPMAP_LINEAR
    );

    sprite(
        const material& mat,
        const sprite_layout* layout = nullptr,
        interpolation mag = interpolation::LINEAR,
        interpolation min = interpolation::LINEAR_MIPMAP_LINEAR
    );

    sprite(const sprite_sheet& sheet, unsigned mode = 0);

    void set_mode(unsigned mode);
    unsigned get_mode() const;

    const sprite_layout* get_layout() const;

    const texture* get_texture() const;
    void set_texture(const texture* tex);
    void set_texture(const texture* tex, const sprite_layout* layout);

    const material& get_material() const;
    void set_material(const material& mat);
    void set_material(const material& mat, const sprite_layout* layout);

    void set_color(vec4 color_factor);
    vec4 get_color() const;

    void set_interpolation(interpolation both);
    void set_interpolation(interpolation mag, interpolation min);

    void set_animation_looping(bool looping);
    bool get_animation_looping() const;

    vec4 get_bounds(const camera& cam, float* angle = nullptr) const;

private:
    unsigned mode;
    bool animation_looping;
    // These are used to select a sampler if they are null in 'mat'.
    interpolation default_mag, default_min;
    material mat;
    const sprite_layout* layout;
};

class LT_API sprite_sheet
{
friend class sprite;
public:
    sprite_sheet(
        const texture* tex = nullptr,
        const sprite_layout& layout = sprite_layout::simple_h(),
        interpolation mag = interpolation::LINEAR,
        interpolation min = interpolation::LINEAR_MIPMAP_LINEAR
    );

    sprite_sheet(
        const material& mat,
        const sprite_layout& layout = sprite_layout::simple_h(),
        interpolation mag = interpolation::LINEAR,
        interpolation min = interpolation::LINEAR_MIPMAP_LINEAR
    );

    sprite_sheet(
        resource_pool& pool,
        const std::string& texture_path,
        const sprite_layout& layout = sprite_layout::simple_h(),
        interpolation mag = interpolation::LINEAR,
        interpolation min = interpolation::LINEAR_MIPMAP_LINEAR
    );

    const texture* get_texture() const;
    void set_texture(const texture* tex);

    void set_interpolation(interpolation both);
    void set_interpolation(interpolation mag, interpolation min);

    void set_animation_looping(bool looping);
    bool get_animation_looping() const;

    const material& get_material() const;
    material& get_material();

    const sprite_layout& get_layout() const;
    sprite_layout& get_layout();

    sprite create_sprite(unsigned mode = 0) const;

private:
    bool animation_looping;
    // These are used to select a sampler if they are null in 'mat'.
    interpolation default_mag, default_min;
    material mat;
    sprite_layout layout;
};

} // namespace lt

#endif

