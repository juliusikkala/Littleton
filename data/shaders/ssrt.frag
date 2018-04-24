#version 400 core

in vec2 uv;
#include "cast_ray.glsl"
#include "deferred_input.glsl"
#include "brdf.glsl"

uniform sampler2D in_linear_depth;
uniform sampler2D in_lighting;
uniform mat4 proj;
uniform float thickness;
uniform float near;
uniform float roughness_cutoff;
uniform float brdf_cutoff;
uniform float ray_offset;
uniform int ray_max_steps;
out vec4 out_color;

// TODO: Parallax envmaps
uniform bool fallback_cubemap;
uniform samplerCube env;
uniform mat4 inv_view;

void main(void)
{
    float roughness, metallic, f0;
    decode_material(uv, roughness, metallic, f0);
    if(roughness > roughness_cutoff) discard;
    roughness = roughness * roughness;

    ivec2 p = ivec2(gl_FragCoord.xy);
    float depth = texelFetch(in_linear_depth, p, 0).x;
    vec3 normal = normalize(decode_normal(uv));
    vec3 o = unproject_position(depth, uv);

    vec3 view = normalize(-o);
    vec3 d = normalize(reflect(-view, normal));

    vec4 surface_color = decode_color_emission(uv);
    vec3 att = brdf_reflection(
        d,
        surface_color.rgb,
        view,
        normal,
        roughness,
        f0,
        metallic
    );
    
    if(max(max(att.r, att.g), att.b) < brdf_cutoff) discard;
    else
    {
        vec2 tp;
        vec3 intersection;
        float fade = min(
            4.0f*cast_ray(
                in_linear_depth, o, d, proj, ray_offset,
                ray_max_steps, thickness, near, tp
            ),
            1.0f
        );

        vec4 color = fallback_cubemap ? 
            texture(env, (inv_view * vec4(-d, 0)).xyz) : 
            vec4(0);
        if(fade == 0.0f && !fallback_cubemap) discard;
        else color = mix(color, texelFetch(in_lighting, ivec2(tp), 0), fade);
        out_color = vec4(color.rgb * att, surface_color.a);
    }
}
