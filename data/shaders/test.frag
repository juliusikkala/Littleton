#version 330 core

in vec2 uv;
out vec4 color;

uniform float time;
uniform sampler2D tex;

void main(void)
{
    vec3 dir = normalize(vec3(uv*2-1, sin(time)+1.0f));
    vec4 tex_color = texture(tex, uv);
    color = TEXTURE_COLOR*tex_color + vec4(dir.xy*0.5f+0.5f, dir.z, 1.0f)*(1-tex_color.a);
}
