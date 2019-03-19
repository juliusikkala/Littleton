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
#include "spherical_gaussians.hh"
#include "helpers.hh"
#include <boost/functional/hash.hpp>

namespace lt
{

bool sg_lobe::operator==(const sg_lobe& other) const
{
    return other.axis == axis && other.sharpness == sharpness;
}

sg_group::sg_group(
    context& ctx,
    uvec3 resolution,
    vec3 size,
    size_t lobe_count,
    float epsilon,
    float max_brightness,
    float near,
    float far
): near(near), far(far), max_brightness(max_brightness)
{
    set_scaling(size*0.5f);

    // Generate amplitude textures
    for(unsigned i = 0; i < lobe_count; ++i)
        amplitudes.emplace_back(
            ctx, resolution, GL_RGBA16F, GL_FLOAT, 0, GL_TEXTURE_3D
        );

    std::vector<vec3> axes;
    
    // Because there are only 2 or 1 lobes, align them in the y-axis
    // (typically the sky and ground are important contributors to radiance)
    if(lobe_count > 0 && lobe_count <= 2)
    {
        if(lobe_count == 2) axes.push_back(vec3(0, -1, 0));
        axes.push_back(vec3(0, 1, 0));
    }
    else axes = packed_sphere_points(lobe_count);

    // Derived by dividing the surface area of a sphere for lobe_count sectors.
    // The gaussian is fit such that its value is epsilon at the edge of the
    // assigned sector.
    float sharpness = log(epsilon) * lobe_count * -0.5f;

    // Add lobes
    for(vec3 axis: axes) lobes.push_back({axis, sharpness});
}

uvec3 sg_group::get_resolution() const
{
    if(amplitudes.size())
    {
        return amplitudes[0].get_dimensions();
    }
    else return uvec3(0);
}

const std::vector<sg_lobe>& sg_group::get_lobes() const
{
    return lobes;
}

texture& sg_group::get_amplitudes(size_t lobe)
{
    return amplitudes[lobe];
}

const texture& sg_group::get_amplitudes(size_t lobe) const
{
    return amplitudes[lobe];
}

float sg_group::get_near() const { return near; }
void sg_group::set_near(float near) { this->near = near; }

float sg_group::get_far() const { return far; }
void sg_group::set_far(float far) { this->far = far; }

float sg_group::get_max_brightness() const { return max_brightness; }

void sg_group::set_max_brightness(float max_brightness)
{
    this->max_brightness = max_brightness;
}

float sg_group::get_density() const
{
    // Number of points in 1x1x1 space.
    vec3 size = get_scaling();
    vec3 res = get_resolution();
    return (res.x*res.y*res.z)/(size.x*size.y*size.z);
}

sg_scene::sg_scene() {}

void sg_scene::add_sg_group(sg_group* group)
{
    sorted_insert(sg_groups, group);
}

void sg_scene::remove_sg_group(sg_group* group)
{
    sorted_erase(sg_groups, group);
}

const std::vector<sg_group*>& sg_scene::get_sg_groups() const
{
    return sg_groups;
}

void sg_scene::clear_sg_groups()
{
    sg_groups.clear();
}

void sg_scene::add_impl(sg_group* group) { add_sg_group(group); }
void sg_scene::remove_impl(sg_group* group) { remove_sg_group(group); }
void sg_scene::clear_impl() { clear_sg_groups(); }

} // namespace lt

size_t boost::hash_value(const lt::sg_lobe& l)
{
    std::size_t seed = 0;
    boost::hash_combine(seed, l.axis.x);
    boost::hash_combine(seed, l.axis.y);
    boost::hash_combine(seed, l.axis.z);
    boost::hash_combine(seed, l.sharpness);
    return seed;
}

