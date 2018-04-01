#version 400 core

in vec3 view_dir;
out vec4 color;

uniform samplerCube skybox;
uniform float exposure;

void main(void)
{
    color = texture(skybox, view_dir) * exposure;
    //color = vec4(normalize(view_dir) * 0.5f + 0.5f, 1.0f);
}
