/* This shader is meant to be used with generic.vert. It is suitable for
 * deferred rendering geometry pass, and supports materials.
 */
#version 330 core

#include "deferred_output.glsl"

void main(void)
{
    vec4 color = get_material_color();

    if(color.a < 0.5f) discard;
    out_color_emission = color;

#ifdef VERTEX_NORMAL
    out_normal = encode_normal(f_normal);
#endif

    out_material = encode_material();
}

