#include "spherical_gaussians.hh"

namespace lt
{

sg_group::sg_group(
    context& ctx,
    uvec3 resolution,
    vec3 size,
    size_t lobe_count,
    float epsilon
){
    set_scaling(size);

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

    // Add lobes and generate amplitude textures
    for(vec3 axis: axes)
    {
        lobes.push_back({axis, sharpness});
        amplitudes.emplace_back(
            ctx, resolution, GL_RGB16F, GL_FLOAT, 0, GL_TEXTURE_3D
        );
    }
}

uvec3 sg_group::get_resolution() const
{
    if(amplitudes.size())
    {
        return amplitudes[0].get_dimensions();
    }
    else return uvec3(0);
}

size_t sg_group::get_lobe_count() const
{
    return lobes.size();
}

sg_lobe sg_group::get_lobe(size_t lobe) const
{
    return lobes[lobe];
}

texture& sg_group::get_amplitudes(size_t lobe)
{
    return amplitudes[lobe];
}

const texture& sg_group::get_amplitudes(size_t lobe) const
{
    return amplitudes[lobe];
}

}
