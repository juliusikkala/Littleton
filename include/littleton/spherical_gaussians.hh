/*
    Copyright 2018-2019 Julius Ikkala

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
#ifndef LT_SPHERICAL_GAUSSIANS_HH
#define LT_SPHERICAL_GAUSSIANS_HH
#include "api.hh"
#include "math.hh"
#include "transformable.hh"
#include "texture.hh"

namespace lt
{

struct LT_API sg_lobe
{
    vec3 axis;
    float sharpness;

    bool operator==(const sg_lobe& other) const;
};

class sg_group;

class LT_API sg_group: public transformable_node
{
friend class sg_probe;
public:
    sg_group(
        context& ctx,
        uvec3 resolution,
        vec3 size,
        size_t lobe_count=12,
        float epsilon = 0.5f,
        float max_brightness = 2.0f,
        float near = 0.001f,
        float far = 100.0f
    );

    sg_group(const sg_group& other) = delete;
    sg_group& operator=(const sg_group& other) = delete;

    uvec3 get_resolution() const;
    const std::vector<sg_lobe>& get_lobes() const;

    texture& get_amplitudes(size_t lobe);
    const texture& get_amplitudes(size_t lobe) const;

    float get_near() const;
    void set_near(float near);

    float get_far() const;
    void set_far(float far);

    // Clamps brightness to this range (makes smoother probes at the expense of
    // realism)
    float get_max_brightness() const;
    void set_max_brightness(float max_brightness);

    float get_density() const;

private:
    std::vector<sg_lobe> lobes;

    std::vector<texture> amplitudes;
    float near, far, max_brightness;
};

class LT_API sg_scene
{
public:
    sg_scene();

    void add_sg_group(sg_group* group);
    void remove_sg_group(sg_group* group);
    const std::vector<sg_group*>& get_sg_groups() const;
    void clear_sg_groups();

    // Glue for composite_scene convenience functions, do not call directly.
    void add_impl(sg_group* group);
    void remove_impl(sg_group* group);
    void clear_impl();

private:
    std::vector<sg_group*> sg_groups;
};

} // namespace lt

namespace boost
{
    size_t hash_value(const lt::sg_lobe& l);
}

#endif

