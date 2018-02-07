/* This shader is gamma correction. */
#version 330 core

uniform sampler2D in_color;
uniform float gamma;
in vec2 uv;
out vec4 out_color;

void main(void)
{
    vec4 c = texture(in_color, uv);
    out_color = vec4(pow(c.rgb, vec3(gamma)), c.a);
}
