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
#ifndef LT_SPHERICAL_GAUSSIANS_HH
#define LT_SPHERICAL_GAUSSIANS_HH
#include "math.hh"
#include "transformable.hh"
#include "texture.hh"

namespace lt
{

struct sg_lobe
{
    vec3 axis;
    float sharpness;
};

class sg_group;

class sg_group: public transformable_node
{
friend class sg_probe;
public:
    sg_group(
        context& ctx,
        uvec3 resolution,
        vec3 size,
        size_t lobe_count=12,
        float epsilon = 0.5f
    );

    uvec3 get_resolution() const;
    size_t get_lobe_count() const;
    sg_lobe get_lobe(size_t lobe) const;

    texture& get_amplitudes(size_t lobe);
    const texture& get_amplitudes(size_t lobe) const;

private:
    std::vector<sg_lobe> lobes;

    std::vector<texture> amplitudes;
};

} // namespace lt

#endif

