#version 430 core
#include "depth.glsl"
#include "deferred_output.glsl"

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

float sphere_distance(vec3 p)
{
    vec3 c = vec3(0.3f);
    vec3 q = mod(p - vec3(0.0f, -2 * time, 0.0f), c)-0.5*c;
    return length(q) - 0.005f;
}

#define OBJECTS \
    X(sphere)

float map(in vec3 p)
{
    float closest = INF;
#define X(name) \
        closest = min(closest, name##_distance(p));
    OBJECTS
#undef X
    return closest;
}

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
    vec3 c = vec3(1);
    if(!intersect(camera_pos, normalize(view_dir), t, n, false)) discard;

    vec3 p = normalize(local_view_dir) * t;
    gl_FragDepth = hyperbolic_depth(p.z)*0.5f+0.5f;

    write_gbuffer(
        p, n_v * n, vec3(1),
        vec3(0), 1.0f, 0.0f, 1.0f
    );

#ifdef APPLY_AMBIENT
    out_lighting = vec4(c * ambient, 0);
#else
    out_lighting = vec4(0);
#endif
}

