#version 410 core

layout(location = 0) in vec3 view_dir;
out vec4 color;

uniform samplerCube skybox;
uniform float exposure;

void main(void)
{
    color = clamp(texture(skybox, view_dir) * exposure, 0.0f, 1000.0f);
}
