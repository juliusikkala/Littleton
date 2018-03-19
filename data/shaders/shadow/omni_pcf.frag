#version 400 core

#include "generic_fragment_input.glsl"

#ifdef DISCARD_ALPHA
#include "material.glsl"
#endif

uniform vec3 pos;
uniform float far_plane;

void main(void)
{
#ifdef DISCARD_ALPHA
    vec4 surface_color = get_material_color();
    if(surface_color.a < DISCARD_ALPHA) discard;
#endif
    
    gl_FragDepth = distance(f_in.position, pos) / far_plane;
}
