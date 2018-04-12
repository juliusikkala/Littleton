#include "projection.glsl"
#include "depth.glsl"
#include "constants.glsl"

void step_texel(inout vec3 s, vec3 d, vec2 level_size)
{
    vec2 o = s.xy * level_size;
    vec2 edge_uv = vec2(
        d.x < 0 ? ceil(o.x) : floor(o.x),
        d.y < 0 ? ceil(o.y) : floor(o.y)
    );

    vec2 t2 = abs((edge_uv - o + sign(d.xy)) / (d.xy * level_size));
    float t = min(t2.x, t2.y);

    t *= 1.01f;

    s += t * d;
}

#define RAY_MIN_LEVEL 0

bool cast_ray(
    in sampler2D linear_depth,
    vec3 origin,
    vec3 dir,
    mat4 proj,
    float ray_steps,
    float thickness,
    float near,
    out vec2 p,
    out vec3 intersection
){
    float len = (origin.z + dir.z > near) ? (near - origin.z) / dir.z : 1.0f; 
    origin += dir * 0.01f;

    vec3 end = origin + dir * len;

    vec4 ps = proj * vec4(origin, 1.0f);
    vec3 s = ps.xyz / ps.w;
    s.xy = s.xy * 0.5f + 0.5f;
    vec3 orig_s = s;

    vec4 pe = proj * vec4(end, 1.0f);
    vec3 e = pe.xyz / pe.w;
    e.xy = e.xy * 0.5f + 0.5f;

    vec3 d = normalize(e - s);

    ivec2 size = textureSize(linear_depth, 0);

    int level = RAY_MIN_LEVEL;
    vec2 level_size = textureSize(linear_depth, level);
    bool hit = false;

    vec3 prev_s = s;

    bool do_step = true;

    while(level > RAY_MIN_LEVEL-1 && level <= RAY_MAX_LEVEL && ray_steps > 0)
    {
        ray_steps -= 1.0f;
        step_texel(s, d, level_size);

        vec2 limit = abs(s.xy * 2.0f - 1.0f);

        hit = false;
        if(max(limit.x, limit.y) < 1.0f)
        {
            p = (prev_s.xy + s.xy)*0.5f;
            vec2 ldepth = texelFetch(
                linear_depth,
                ivec2(p*level_size),
                level
            ).rg;

            float depth_hi = hyperbolic_depth(ldepth.g);
            float depth_lo = hyperbolic_depth(ldepth.r - thickness);

            if(isnan(depth_hi)) depth_hi = 10000.0f;
            if(isnan(depth_lo)) depth_lo = 10000.0f;

            vec2 s_depth = vec2(prev_s.z, s.z);
            if(s_depth.x > s_depth.y) s_depth = s_depth.yx;

            if(s_depth.x < depth_lo && s_depth.y > depth_hi)
            {// Hit
                s = prev_s;
                level--;
                level_size = size >> level;
                hit = true;
            }
            else
            {// Miss
                level++;
                level_size = size >> level;
            }
            prev_s = s;
        }
        else if(level != RAY_MIN_LEVEL)
        {
            s = prev_s;
            level--;
            level_size = size >> level;
        }
        else break;
    }

    return level == RAY_MIN_LEVEL-1 && hit;
}
