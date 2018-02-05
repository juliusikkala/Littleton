#include "constants.glsl"
#include "generic_fragment_input.glsl"
#include "material.glsl"

layout(location=0) out vec4 out_color_emission;

#ifdef VERTEX_NORMAL
layout(location=1) out vec2 out_normal;
#endif

layout(location=2) out vec4 out_material;

vec2 encode_normal(vec3 normal)
{
    vec3 n = normalize(normal);
    return inversesqrt(2.0f+2.0f*n.z)*n.xy;
}

vec4 encode_material()
{
    return vec4(
        get_material_roughness(),
        get_material_metallic(),
        (get_material_ior()-1.0f)/4.0f,
        0.0f
    );
}
