#version 430 core

#include "generic_fragment_input.glsl"

uniform samplerCube cubemap;
out vec4 color;

void main(void)
{
    color = texture(cubemap, f_in.position);
}

