#version 400 core

in vec2 uv;

out vec4 color;
uniform sampler2D src1;
uniform float multiplier1;
uniform sampler2D src2;
uniform float multiplier2;

void main(void)
{
    color = texture(src1, uv) * multiplier1 + texture(src2, uv) * multiplier2;
}
