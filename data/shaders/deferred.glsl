#include "constants.glsl"
#include "generic_fragment_input.glsl"
#include "material.glsl"

layout(location=0) out vec4 color_emission;

#ifdef VERTEX_NORMAL
layout(location=1) out vec2 normal;
#endif

vec2 encode_normal(vec3 normal)
{
    vec3 n = normalize(normal);
    return inversesqrt(2.0f+2.0f*n.z)*n.xy;
}

vec3 decode_normal(vec2 n)
{
    vec2 n2 = n*SQRT2;
    float d = dot(n2, n2);
    float f = sqrt(2 - d);
    return vec3(f*n2, 1 - d);
}

