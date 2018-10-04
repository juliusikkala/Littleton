#version 400 core

in vec2 uv;
uniform mat4 proj;

#include "ssrt.glsl"
out vec4 out_color;

void main(void)
{
    material_t mat;
    decode_material(uv, mat);

    ivec2 p = ivec2(gl_FragCoord.xy);
    float depth = texelFetch(in_linear_depth, p, 0).x;
    vec3 o = unproject_position(depth, uv);

    vec3 view = normalize(-o);

    vec3 r = ssrt_reflection(view, mat, o);
    if(r == vec3(0)) discard;

    out_color = vec4(r, 1.0f);
}
