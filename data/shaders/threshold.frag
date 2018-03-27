#version 400 core

in vec2 uv;
uniform sampler2D src_color;
uniform float threshold;
uniform vec4 fail_color;

out vec4 out_color;

void main(void)
{
    vec4 color = texture(src_color, uv).rgba;

    if(color.r < threshold && color.g < threshold && color.b < threshold)
        color = fail_color;

    out_color = color;
}
