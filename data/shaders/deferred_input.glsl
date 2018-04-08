#include "constants.glsl"
#include "depth.glsl"
#include "projection.glsl"

uniform sampler2D in_color_emission;
uniform sampler2D in_normal;
uniform sampler2D in_material;

vec4 decode_color_emission(vec2 uv)
{
    vec4 d = texture(in_color_emission, uv);
    return vec4(d.rgb, 1.0f/d.w - 1.0f);
}

vec3 decode_color_only(vec2 uv)
{
    return texture(in_color_emission, uv).rgb;
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

