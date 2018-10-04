#include "constants.glsl"
#include "depth.glsl"
#include "projection.glsl"
#include "material.glsl"

uniform sampler2D in_color;
uniform sampler2D in_normal;
uniform sampler2D in_material;
uniform sampler2D in_lighting;
uniform sampler2D in_indirect_lighting;

vec3 decode_color(vec2 uv)
{
    return texture(in_color, uv).rgb;
}

vec3 decode_normal(vec2 uv)
{
    return unproject_lambert_azimuthal_equal_area(texture(in_normal, uv).xy);
}

void decode_material(
    vec2 uv,
    out float roughness,
    out float metallic,
    out float f0
){
    vec4 encoded = texture(in_material, uv);
    roughness = encoded.x;
    metallic = encoded.y;
    // Instead of deferred_output.glsl, f0 is multiplied by 2 in material.cc
    // in order to use most of the available range and resolution.
    f0 = encoded.z/2.0f;
}

vec3 decode_position(vec2 uv)
{
    return unproject_position(get_linear_depth(uv), uv);
}

vec3 decode_lighting(vec2 uv)
{
    return texture(in_lighting, uv).rgb;
}

vec3 decode_indirect_lighting(vec2 uv)
{
    return texture(in_indirect_lighting, uv).rgb;
}

void decode_material(vec2 uv, out material_t mat)
{
    mat.color = vec4(decode_color(uv), 1.0f);
    decode_material(uv, mat.roughness, mat.metallic, mat.f0);
    mat.normal = decode_normal(uv);
    mat.emission = vec3(0); // This can't be reconstructed unfortunately
}
