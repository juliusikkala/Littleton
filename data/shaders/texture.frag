#version 330 core

in vec2 uv;
out vec4 color;

uniform sampler2D tex;

void main(void)
{
    color = vec4(texture(tex, uv).xyz, 1.0f);
}

