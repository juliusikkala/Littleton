#version 430 core

uniform mat4 proj;
uniform mat4 view;

#include "depth.glsl"
#include "deferred_output.glsl"

#ifdef USE_SSRT
#include "ssrt.glsl"
#endif

layout(location = 0) in vec3 view_dir;
layout(location = 1) in vec3 local_view_dir;

#ifdef APPLY_AMBIENT
uniform vec3 ambient;
#endif

uniform vec3 camera_pos;
uniform mat3 n_v;
uniform int num_refractions;
uniform int num_reflections;
uniform int max_steps;
uniform float min_dist;
uniform float max_dist;
uniform float step_ratio;
uniform float hit_ratio;
uniform float time;

SDF_INSERT_CODE

// World-space normal
vec3 normal(vec3 p)
{
    const float eps = 0.0001f;
    const vec2 k = vec2(1.0f, -1.0f);
    return normalize(
        k.xyy * map(p + k.xyy * eps) +
        k.yyx * map(p + k.yyx * eps) +
        k.yxy * map(p + k.yxy * eps) +
        k.xxx * map(p + k.xxx * eps)
    );
}

bool intersect(
    in vec3 o,
    in vec3 v,
    out float t,
    out vec3 n,
    float min_dist,
    bool inside
) {
    t = min_dist;
    float dir = inside ? -1.0f : 1.0f;
    float dist = 0;

    for(int i = 0; i < max_steps; ++i)
    {
        vec3 p = o + t * v;
        dist = dir * map(p);
        if(dist < hit_ratio * t || t > max_dist) break;
        t += dist * step_ratio;
    }

    n = normal(o + t * v);

    return dist < hit_ratio * t;
}

void main(void)
{
    float t;
    vec3 n;
    vec3 v = normalize(view_dir);
    vec3 lv = normalize(local_view_dir);
    material_t mat;
    mat.color = vec4(vec3(1), 0.0f);
    mat.metallic = 0.0f;
    mat.roughness = 0.01f;
    mat.f0 = 1.3f;
    if(!intersect(camera_pos, v, t, n, min_dist, false))
        discard;

    float eta = mat.f0;
    mat.f0 = (eta-1)/(eta+1);
    mat.f0 *= mat.f0;
    mat.normal = n_v * n;

    vec3 p = lv * t;
    gl_FragDepth = hyperbolic_depth(p.z)*0.5f+0.5f;

    write_gbuffer(
        p, mat.normal, mat.color.rgb * mat.color.a,
        vec3(0), mat.roughness, mat.metallic, mat.f0
    );

    vec3 lighting = vec3(0);
#ifdef USE_SSRT
    lighting += ssrt_reflection(-lv, mat, p);

    // Refraction ray
    if(mat.color.a < 1.0f)
    {
        vec3 rp = camera_pos + v * t;
        vec3 rrv = normalize(refract(v, n, 1.0f/eta));
        vec3 rn;
        float rt;

        intersect(rp, rrv, rt, rn, 0.1f, true);

        vec3 rv = normalize((view * vec4(rrv, 0)).xyz);
        p += rv * rt;

        mat.normal = normalize(n_v * rn);
        lighting += ssrt_refraction(rv, mat, p, eta);
    }
#endif

#ifdef APPLY_AMBIENT
    out_lighting = vec4(
        lighting + (1.0f - mat.metallic) * mat.color.rgb * mat.color.a * ambient,
        1.0f
    );
#else
    out_lighting = vec4(lighting, 1.0f);
#endif
}

