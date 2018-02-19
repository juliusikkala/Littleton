#version 400 core

in vec2 uv;
out vec4 color;

uniform float time;

void main(void)
{
    vec3 dir = normalize(vec3(uv*2-1, 0.5f));
    dir = vec3(dir.xy*0.5f+0.5f, dir.z);
    color = vec4(dir, 1.0f);
}
