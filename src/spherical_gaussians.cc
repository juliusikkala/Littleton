#include "spherical_gaussians.hh"

namespace lt
{

sg_probe::sg_probe(
    const std::vector<sg_lobe>& lobes,
    vec3* amplitudes,
    vec3& pos
): lobes(lobes), amplitudes(amplitudes), pos(pos) {}

const std::vector<sg_lobe>& sg_probe::get_lobes() const
{
    return lobes;
}

vec3 sg_probe::get_position() const
{
    return pos;
}

void sg_probe::set_position(vec3 pos) const
{
    this->pos = pos;
}

vec3 sg_probe::operator[](size_t i) const
{
    return amplitudes[i];
}

vec3& sg_probe::operator[](size_t i)
{
    return amplitudes[i];
}

sg_group::sg_group(size_t lobe_count, float epsilon)
{
    // Always add DC entry (if we have any lobes at all)
    if(lobe_count > 0)
    {
        // The direction here doesn't matter, since sharpness == 0
        // => the gaussian is constant, equal to its amplitude. Just set the
        // direction to something other than zero to avoid dividing by a
        // zero-length vector somewhere.
        lobes.push_back({vec3(1,0,0), 0.0f});
        lobe_count--;
    }

    std::vector<vec3> axes;
    
    // Because there are only 2 or 1 lobes, align them in the y-axis
    // (typically the sky and ground are important contributors to radiance)
    if(lobe_count <= 2)
    {
        if(lobe_count == 2) axes.push_back(vec3(0, -1, 0));
        axes.push_back(vec3(0, 1, 0));
    }
    else axes = packed_sphere_points(lobe_count);

    // Derived by dividing the surface area of a sphere for lobe_count sectors.
    // The gaussian is fit such that its value is epsilon at the edge of the
    // assigned sector.
    float sharpness = log(epsilon) * lobe_count * -0.5f;

    for(vec3 axis: axes) lobes.push_back({axis, sharpness});
}

sg_group::sg_group(const std::vector<sg_lobe>& lobes)
: lobes(lobes) {}

sg_probe sg_group::add_probe(vec3 pos)
{
    size_t new_size = amplitudes.size() + lobes.size();
    amplitudes.resize(new_size, vec3(0.0f));
    probes.push_back(pos);

    return sg_probe(
        lobes,
        &amplitudes[lobes.size() * (probes.size() - 1)],
        probes.back()
    );
}

void sg_group::remove_probe(const sg_probe& probe)
{
    remove_probe(&probe.pos - probes.data());
}

void sg_group::remove_probe(size_t i)
{
    probes.erase(probes.begin() + i);
    auto start = amplitudes.begin() + i * lobes.size();
    amplitudes.erase(start, start + lobes.size());
}

void sg_group::clear()
{
    amplitudes.clear();
    probes.clear();
}

size_t sg_group::probe_count() const
{
    return probes.size();
}

sg_probe sg_group::operator[](size_t i)
{
    return sg_probe(lobes, &amplitudes[lobes.size() * i], probes[i]);
}

}
