#include "constants.glsl"
uniform sampler2D in_depth_stencil;
uniform sampler2D in_color_emission;
uniform sampler2D in_normal;
uniform sampler2D in_material;

uniform vec4 perspective_data;

in vec2 uv;

float get_depth()
{
    return texture(in_depth_stencil, uv).x;
}

vec3 decode_position()
{
    float depth = texture(in_depth_stencil, uv).x * 2.0f - 1.0f;
    float n = perspective_data.z;
    float f = perspective_data.w;
    // Linearize depth
    depth = 2 * n * f / (n + f - depth * (f - n));
    return vec3((0.5f-uv)*perspective_data.xy*depth, depth);
}

vec3 decode_normal()
{
    vec2 n2 = texture(in_normal, uv).xy * SQRT2;
    float d = dot(n2, n2);
    float f = sqrt(2.0f - d);
    return vec3(f*n2, 1.0f - d);
}

void decode_material(out float roughness, out float metallic, out float ior)
{
    vec4 encoded = texture(in_material, uv);
    roughness = encoded.x;
    metallic = encoded.y;
    ior = encoded.z*4.0f+1.0f;
}

vec3 get_albedo()
{
    return texture(in_color_emission, uv).rgb;
}
