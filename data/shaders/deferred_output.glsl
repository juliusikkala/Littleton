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
