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
    float depth = texelFetch(in_linear_depth, p, 0).y;
    vec3 normal = normalize(decode_normal(uv));
    vec3 o = unproject_position(depth, uv);
    vec3 view = normalize(-o);
    vec3 d = normalize(reflect(-view, normal));

    vec2 tp;
    vec3 intersection;
    bool hit = cast_ray(
        in_linear_depth, o, d, proj,
        ray_max_steps, thickness, near,
        tp, intersection
    );

    vec2 abs_tp = abs(tp*2.0f - 1.0f);
    float fade = min((1.0f - max(abs_tp.x, abs_tp.y)), 1.0f);

    out_color = hit ?
        fade * texelFetch(
            in_lighting,
            ivec2(tp * textureSize(in_lighting, 0)),
            0
        ) :
        vec4(0);

    //out_color = hit ? vec4(0,1,0,0) : vec4(1,0,0,0);
}
