#include "projection.glsl"
#include "depth.glsl"
#include "constants.glsl"

#define RAY_MIN_LEVEL 0

void step_ray(out vec2 p, inout vec3 s, vec3 d, vec2 sd, vec2 level_size)
{
    p = s.xy * level_size;

    vec2 t2 = (floor(p) + sd - p) / (d.xy * level_size);
    float t = min(t2.x, t2.y);

    s += t * d;
}

float cast_ray(
    in sampler2D linear_depth,
    vec3 origin,
    vec3 dir,
    mat4 proj,
    float ray_steps,
    float thickness,
    float near,
    out vec2 p
){
    float len = (origin.z + dir.z > near) ? (near - origin.z) / dir.z : 1.0f; 

    vec3 end = origin + dir * len;

    vec4 ps = proj * vec4(origin, 1.0f);
    vec3 s = ps.xyz / ps.w;

    vec4 pe = proj * vec4(end, 1.0f);
    vec3 e = pe.xyz / pe.w;

    vec3 d = e - s;
    d.z *= 2.0f;
    s.xy = s.xy * 0.5f + 0.5f;

    ivec2 size = textureSize(linear_depth, 0);

    int level = RAY_MIN_LEVEL;
    vec2 level_size = size >> level;

    vec3 prev_s = s;

    vec2 sd = 0.5f + 0.501f * vec2(
        d.x < 0 ? -1 : 1,
        d.y < 0 ? -1 : 1
    );

    vec2 depth = vec2(0.0f);

    float hyperbolize_mul = -2.0f * clip_info.x/clip_info.y;
    float hyperbolize_constant = -clip_info.z/clip_info.y;

    for(int i = 0; i < 3; ++i) step_ray(p, s, d, sd, level_size);

    while(
        level >= RAY_MIN_LEVEL &&
        level <= RAY_MAX_LEVEL &&
        s.z < 1.0f &&
        ray_steps > 0
    ){
        ray_steps-=1.0f;

        prev_s = s;
        step_ray(p, s, d, sd, level_size);

#ifdef INFINITE_THICKNESS
        depth.x = texelFetch(linear_depth, ivec2(p), level).x;
        depth.x = hyperbolize_mul/depth.x + hyperbolize_constant;

        if(max(prev_s.z, s.z) >= depth.x)
        {// Hit
            s = prev_s;
            level--;
        }
        else level++; // Miss
#else
        depth = texelFetch(linear_depth, ivec2(p), level).xy;
        depth.y -= thickness;
        depth = hyperbolize_mul/depth + hyperbolize_constant;

        vec2 s_depth = vec2(prev_s.z, s.z);
        if(s_depth.x > s_depth.y) s_depth = s_depth.yx;

        if(s_depth.x < depth.y && s_depth.y > depth.x)
        {// Hit
            s = prev_s;
            level--;
        }
        else level++; // Miss
#endif

        level_size = (size >> level);
    }

    vec3 fade = abs(vec3(prev_s.xy * 2.0f, prev_s.z) - 1.0f);
#ifdef INFINITE_THICKNESS
    float hit_fade = clamp(1.0f - (s.z - depth.x)*300.f, 0.0f, 1.0f);
#else
    float hit_fade = 1.0f;
#endif

    return level == RAY_MIN_LEVEL-1 ?
        (1.0f - max(max(fade.x, fade.y), fade.z)) * hit_fade:
        0.0f;
}
