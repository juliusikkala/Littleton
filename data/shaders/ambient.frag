#version 400 core

#include "deferred_input.glsl"

uniform sampler2D occlusion;
uniform vec3 ambient;

out vec3 out_color;

void main(void)
{
    vec4 surface_color = decode_color_emission(uv);

    out_color = surface_color.rgb * ambient * texture(occlusion, uv).x;
}
