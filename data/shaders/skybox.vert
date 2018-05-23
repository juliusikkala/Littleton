#version 400 core

layout(location = 0) in vec2 vertex;
out vec3 view_dir;

uniform mat4 projection;
uniform mat4 inv_view;

void main(void)
{
    gl_Position = vec4(vertex, 0.0f, 1.0f);
    vec4 pp = projection * vec4(vertex, -1.0f, 1.0f);
    view_dir = (inv_view * vec4(pp.xyz, 0)).xyz;
}

