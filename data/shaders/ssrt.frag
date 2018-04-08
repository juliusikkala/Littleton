#version 400 core

#include "cast_ray.glsl"
#include "deferred_input.glsl"

in vec2 uv;
uniform sampler2D in_linear_depth;
uniform sampler2D in_lighting;
out vec4 out_color;

void main(void)
{
    ivec2 p = ivec2(gl_FragCoord.xy);
    float depth = texelFetch(in_linear_depth, p, 0).x;
    vec3 normal = normalize(decode_normal(uv));
    vec3 o = unproject_position(depth, uv);
    vec3 view = normalize(-o);
    vec3 d = reflect(-view, normal);

    vec2 hit_uv = uv;
    bool hit = cast_ray(in_linear_depth, uv, o, d, hit_uv);
    
    out_color = hit ? texture(in_lighting, hit_uv) : vec4(0);
}
