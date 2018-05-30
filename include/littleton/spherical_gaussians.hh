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

namespace lt
{

struct sg_lobe
{
    vec3 axis;
    float sharpness;
};

class sg_group;

class sg_probe
{
friend class sg_group;
public:
    const std::vector<sg_lobe>& get_lobes() const;
    vec3 get_position() const;
    void set_position(vec3 pos) const;

    vec3 operator[](size_t i) const;
    vec3& operator[](size_t i);

private:
    sg_probe(
        const std::vector<sg_lobe>& lobes,
        vec3* amplitudes,
        vec3& pos
    );

    const std::vector<sg_lobe>& lobes;
    vec3* amplitudes;
    vec3& pos;
};


class sg_group: public transformable_node
{
friend class sg_probe;
public:
    sg_group(size_t lobe_count=12, float epsilon = 0.5f);
    explicit sg_group(const std::vector<sg_lobe>& lobes);

    // Invalidates all existing sg_probes!
    sg_probe add_probe(vec3 pos);

    // Invalidates all existing sg_probes!
    void remove_probe(const sg_probe& probe);

    // Invalidates all existing sg_probes!
    void remove_probe(size_t i);

    void clear();

    size_t probe_count() const;

    sg_probe operator[](size_t i);

private:
    std::vector<sg_lobe> lobes;
    std::vector<vec3> amplitudes; // size = lobes.size() * probes.size()
    std::vector<vec3> probes;
};

} // namespace lt

#endif

