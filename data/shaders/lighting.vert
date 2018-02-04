#version 330 core

layout(location = 0) in vec2 vertex;

out vec2 uv;

void main(void)
{
    uv = vertex.xy * 0.5f + 0.5f;
    gl_Position = vec4(vertex, 0, 1.0f);
}
