#version 400 core

#include "depth.glsl"

in vec2 uv;
out float depth;

void main(void)
{
    depth = get_linear_depth(uv);
}
