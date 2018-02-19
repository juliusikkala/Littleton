#version 400 core

#include "generic_fragment_input.glsl"

uniform sampler2D tex;
out vec4 out_color;

void main(void)
{
    out_color = texture(tex, f_in.uv);
}
