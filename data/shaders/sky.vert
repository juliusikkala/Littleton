#version 400 core

layout(location = 0) in vec2 vertex;
layout(location = 3) in vec2 v_uv;
out vec3 pos;
out vec2 uv;

uniform mat4 ip;

void main(void)
{
    gl_Position = vec4(vertex, 0.0f, 1.0f);
    pos = (ip * vec4(vertex, -1.0f, 1.0f)).xyz;
    uv = v_uv;
}
