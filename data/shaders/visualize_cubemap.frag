#version 430 core

#include "generic_fragment_input.glsl"

#ifdef CUBEMAP_ARRAY
uniform samplerCubeArray cubemap;
uniform int array_index;
#else
uniform samplerCube cubemap;
#endif
out vec4 color;

void main(void)
{
#ifdef CUBEMAP_ARRAY
    vec4 s = texture(cubemap, vec4(f_in.position, array_index));
#else
    vec4 s = texture(cubemap, f_in.position);
#endif
    color = clamp(s, vec4(0), vec4(1000));
}

