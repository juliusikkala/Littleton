#version 400 core

#include "deferred_input.glsl"

uniform sampler2D occlusion;
uniform vec3 ambient;

in vec2 uv;
out vec3 out_color;

void main(void)
{
    vec3 surface_color = decode_color_only(uv);

    out_color = surface_color * ambient * texture(occlusion, uv).x;
}
