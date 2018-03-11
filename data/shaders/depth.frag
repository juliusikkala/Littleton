#version 400 core

#ifdef DISCARD_ALPHA
#include "generic_fragment_input.glsl"
#include "material.glsl"
#endif

void main(void)
{
#ifdef DISCARD_ALPHA
    vec4 surface_color = get_material_color();
    if(surface_color.a < DISCARD_ALPHA) discard;
#endif
}
