#version 330 core

in vec2 uv;
out vec4 out_color;

void main(void)
{
    vec3 dir = normalize(vec3(uv.xy*2.0f-1.0f, 0.5f));
    out_color = vec4(dir.xy*0.5f+0.5f, dir.z, 1.0f);
}

