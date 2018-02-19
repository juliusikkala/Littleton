/* This shader is meant to be used with generic.vert. It is suitable for
 * deferred rendering geometry pass, and supports materials.
 */
#version 400 core

#include "deferred_output.glsl"

void main(void)
{
    vec4 color = get_material_color();

    if(color.a < 0.5f) discard;
    out_color_emission = color;

#ifdef VERTEX_NORMAL
    vec3 normal = normalize(f_in.normal);

#if defined(MATERIAL_NORMAL_TEXTURE) && defined(VERTEX_TANGENT)
    mat3 tbn = mat3(normalize(f_in.tangent), normalize(f_in.bitangent), normal);
    vec3 ts_normal = get_material_normal();
    normal = tbn * ts_normal;
#endif

    out_normal = encode_normal(normal);
#endif

    out_material = encode_material();
}

