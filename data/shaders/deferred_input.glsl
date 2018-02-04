#include "constants.glsl"
uniform sampler2D depth_stencil;
uniform sampler2D color_emission;
uniform sampler2D normal;

uniform vec4 perspective_data;

in vec2 uv;

vec3 decode_position()
{
    float depth = texture(depth_stencil, uv).x * 2.0f - 1.0f;
    float n = perspective_data.z;
    float f = perspective_data.w;
    // Linearize depth
    depth = 2 * n * f / (n + f - depth * (f - n));
    return vec3((0.5f-uv)*perspective_data.xy*depth, depth);
}

vec3 decode_normal()
{
    vec2 n2 = texture(normal, uv).xy * SQRT2;
    float d = dot(n2, n2);
    float f = sqrt(2.0f - d);
    return vec3(f*n2, 1.0f - d);
}

vec3 get_albedo()
{
    return texture(color_emission, uv).rgb;
}
