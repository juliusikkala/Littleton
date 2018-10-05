#include "cast_ray.glsl"
#include "deferred_input.glsl"
#include "brdf.glsl"

uniform float ssrt_thickness;
uniform float near;
uniform float ssrt_roughness_cutoff;
uniform float ssrt_brdf_cutoff;
uniform float ssrt_ray_offset;
uniform int ssrt_ray_max_steps;

// TODO: Parallax envmaps
#ifdef FALLBACK_CUBEMAP
uniform samplerCube ssrt_env;
uniform float ssrt_env_exposure;
uniform mat4 ssrt_env_inv_view;
#endif

vec3 ssrt_reflection(
    vec3 view,
    in material_t mat,
    vec3 o
){
    if(mat.roughness > ssrt_roughness_cutoff) return vec3(0);

    ivec2 p = ivec2(gl_FragCoord.xy);

    vec3 d = normalize(reflect(-view, mat.normal));

    vec3 att = brdf_reflection(
        d,
        mat.color.rgb,
        view,
        mat.normal,
        mat.roughness,
        mat.f0,
        mat.metallic
    );
    
    if(max(max(att.r, att.g), att.b) < ssrt_brdf_cutoff) return vec3(0);
    else
    {
        vec2 tp;
        vec3 intersection;
        float fade = min(
            4.0f*cast_ray(
                in_linear_depth, o, d, proj, ssrt_ray_offset,
                ssrt_ray_max_steps, ssrt_thickness, near, tp
            ),
            1.0f
        );

#ifdef FALLBACK_CUBEMAP
        vec4 color = texture(
            ssrt_env,
            (ssrt_env_inv_view * vec4(d, 0)).xyz
        ) * ssrt_env_exposure;
        color = mix(color, texelFetch(in_lighting, ivec2(tp), 0), fade);
#else
        if(fade == 0.0f) return vec3(0);
        vec4 color = texelFetch(in_lighting, ivec2(tp), 0) * fade;
#endif
        return color.rgb * att;
    }
}

vec3 ssrt_refraction(
    vec3 view,
    in material_t mat,
    vec3 o,
    float eta
){
    if(mat.roughness > ssrt_roughness_cutoff) return vec3(0);

    ivec2 p = ivec2(gl_FragCoord.xy);

    vec3 d = normalize(refract(view, -mat.normal, eta));

    vec2 tp;
    vec3 intersection;
    float fade = clamp(
        4.0f*cast_ray(
            in_linear_depth, o, d, proj, ssrt_ray_offset,
            ssrt_ray_max_steps, ssrt_thickness, near, tp
        ),
        0.0f,
        1.0f
    );

#ifdef FALLBACK_CUBEMAP
    vec4 color = texture(
        ssrt_env,
        (ssrt_env_inv_view * vec4(d, 0)).xyz
    ) * ssrt_env_exposure;
    color = mix(color, texelFetch(in_lighting, ivec2(tp), 0), fade);
#else
    if(fade == 0.0f) return vec3(0);
    vec4 color = texelFetch(in_lighting, ivec2(tp), 0) * fade;
#endif
    return color.rgb;
}
