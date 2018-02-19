/* This shader applies a filmic tonemapping algorithm to the image. */
#version 400 core

uniform sampler2D in_color;
uniform float exposure;
in vec2 uv;
out vec3 out_color;

void main(void)
{
    vec3 c = texture(in_color, uv).rgb * exposure;
    c = max(vec3(0.0f), c-0.004f);
    out_color = (c * (6.2f * c + 0.5f))/(c * (6.2f * c + 1.7f) + 0.06f);
}
