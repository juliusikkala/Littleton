#include "projection.glsl"
#include "depth.glsl"

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

    s += (1 << RAY_MIN_LEVEL) * 0.005f * d;
    vec3 prev_s = s;
    prev_s.z = -1;

    bool do_step = true;

    while(level > RAY_MIN_LEVEL-1 && level < RAY_MAX_LEVEL && ray_steps > 0)
    {
        ray_steps -= 1.0f;
        step_texel(s, d, level_size);

        vec2 limit = abs(s.xy * 2.0f - 1.0f);

        if(max(limit.x, limit.y) < 1.0f)
        {
            vec2 ldepth = texelFetch(
                linear_depth,
                ivec2(s.xy*level_size),
                level
            ).rg;

            float depth_hi = hyperbolic_depth(ldepth.g);
            float depth_lo = hyperbolic_depth(ldepth.r - thickness);

            vec2 s_depth = vec2(prev_s.z, s.z);
            if(s_depth.x > s_depth.y) s_depth = s_depth.yx;

            if(s_depth.x < depth_lo && s_depth.y > depth_hi)
            {
                level--;
            }
            prev_s = s;
        }
        else break;

        /*ray_steps -= 1.0f;

        if(do_step) step_texel(s, d, screen_p, level_size);
        do_step = true;

        p = screen_p / level_size;
        float depth = hyperbolic_depth(
            texelFetch(linear_depth, ivec2(screen_p), level).g
        );

        if(s.z > depth)
        {// Hit
            float t = (s.z - depth) / d.z;
            s -= t * d;

            level--;
            level_size = textureSize(linear_depth, level);
            screen_p = floor(s.xy * level_size) + 0.5f;
        }
        else if(level < 2)
        {// Miss
            level++;
            level_size = textureSize(linear_depth, level);
            screen_p = floor(s.xy * level_size) + 0.5f;
        }
        prev_s = s;*/

        /*
        ray_steps -= 1.0f;

        step_texel(s, d, screen_p, level_size);

        vec2 limit = abs(s.xy * 2.0f - 1.0f);

        if(max(limit.x, limit.y) < 1.0f)
        {
            vec2 ldepth = texelFetch(linear_depth, ivec2(screen_p), level).rg;

            float depth_hi = hyperbolic_depth(ldepth.r);
            float depth_lo = hyperbolic_depth(ldepth.g - thickness);

            vec2 s_depth = vec2(prev_s.z, s.z);
            if(s_depth.x > s_depth.y) s_depth = s_depth.yx;

            if(s_depth.x > depth_hi && s_depth.y < depth_lo)
            {
                //float t = (s.z - depth_hi) / d.z;
                //s -= d * t;
                s = prev_s;
                level--;
                if(level != -1)
                {
                    level_size = size >> level;
                    screen_p = floor(s.xy * level_size) + 0.5f;
                }
            }
            else
            {
                level++;
                level_size = size >> level;
                screen_p = floor(s.xy * level_size) + 0.5f;
            }
            prev_s = s;

            //screen_p = floor(s.xy * level_size) + 0.5f;
        }
        else break;
        */
    }
    p = s.xy;

    return level == RAY_MIN_LEVEL-1;
    //return false;
}
