#version 400 core

in vec2 uv;
#include "cast_ray.glsl"
#include "deferred_input.glsl"

uniform sampler2D in_linear_depth;
uniform sampler2D in_lighting;
uniform mat4 proj;
uniform float thickness;
uniform float near;
uniform int ray_max_steps;
out vec4 out_color;

void main(void)
{
    ivec2 p = ivec2(gl_FragCoord.xy);
    float depth = texelFetch(in_linear_depth, p, 0).x;
    vec3 normal = normalize(decode_normal(uv));
    vec3 o = unproject_position(depth, uv);
    vec3 view = normalize(-o);
    vec3 d = reflect(-view, normal);

    vec2 tp;
    vec3 intersection;
    float fade = min(
        4.0f*cast_ray(
            in_linear_depth, o, d, proj,
            ray_max_steps, thickness, near,
            tp, intersection
        ),
        1.0f
    );

    out_color =
        fade != 0.0f ?
        fade * texelFetch(in_lighting, ivec2(tp), 0) :
        vec4(0);
}
