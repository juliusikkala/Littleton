#version 400 core

#include "generic_fragment_input.glsl"
#include "constants.glsl"

#ifdef DISCARD_ALPHA
#define USE_INPUT_MATERIAL
#include "material.glsl"
#endif

uniform vec3 pos;
uniform float far_plane;

out vec4 out_color;

void main(void)
{
#ifdef DISCARD_ALPHA
    vec4 surface_color = get_material_color();
    if(surface_color.r < DISCARD_ALPHA) discard;
#endif

    float z = (distance(f_in.position, pos)/far_plane) * 2.0f - 1.0f;
    float z2 = z * z;
    float z3 = z2 * z;
    float z4 = z2 * z2;
    mat4 q = mat4(
        1.5f, 0.0f, SQRT3/2.0f, 0.0f,
        0.0f, 4.0f, 0.0f, 0.5f,
        -2.0f, 0.0f, -SQRT3*2.0f/9.0f, 0.0f,
        0.0f, -4.0f, 0.0f, 0.5f
    );
    out_color = q * vec4(z, z2, z3, z4) + vec4(0.5f, 0.0f, 0.5f, 0.0f);
}

