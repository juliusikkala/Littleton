/* This shader is meant to be used with generic.vert. It is suitable for
 * deferred rendering geometry pass, and supports materials.
 */
#version 330 core

#include "deferred.glsl"

void main(void)
{
    vec4 color = get_material_color();

    if(color.a < 0.5f) discard;
    color_emission = color;

#ifdef VERTEX_NORMAL
    normal = encode_normal(f_normal);
#endif
}

