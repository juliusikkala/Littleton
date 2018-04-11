#include "projection.glsl"
#include "depth.glsl"

#define CELL_STEP_OFFSET 0.05

void step_texel(inout vec3 s, vec3 d, inout vec2 screen_p, vec2 level_size)
{
    vec2 p = s.xy * level_size;
    vec2 o = p - screen_p;

    vec2 sd = sign(d.xy);
    vec2 t2 = (0.5f * sd - o) / (d.xy * level_size);

    float t = t2.y;

    if(t2.x < t2.y)
    {
        sd.y = 0;
        t = t2.x;
    }
    else if(t2.x > t2.y)
    {
        sd.x = 0;
    }
    
    screen_p += sd;
    s += t * d;
}

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

    vec4 pe = proj * vec4(end, 1.0f);
    vec3 e = pe.xyz / pe.w;
    e.xy = e.xy * 0.5f + 0.5f;

    vec3 d = normalize(e - s);

    ivec2 size = textureSize(linear_depth, 0);

    int level = 0;
    vec2 level_size = size >> level;
    bool hit = false;


    s += 0.001f * d;
    vec2 screen_p = floor(s.xy * level_size) + 0.5f;
    vec3 prev_s = s;
    prev_s.z = -1;

    while(level > -1 && level < RAY_MAX_LEVEL && ray_steps > 0)
    {
        ray_steps -= 1.0f;
        step_texel(s, d, screen_p, level_size);

        vec2 ldepth = texelFetch(linear_depth, ivec2(screen_p), level).rg;

        float depth_hi = hyperbolic_depth(ldepth.r);
        float depth_lo = hyperbolic_depth(ldepth.g - thickness);

        vec2 s_depth = vec2(prev_s.z, s.z);
        if(s_depth.x > s_depth.y) s_depth = s_depth.yx;

        if(s_depth.y > depth_hi && s_depth.x < depth_lo)
        {
            p = screen_p / level_size;
            level--;
        }
        prev_s = s;

        /*ray_steps -= 1.0f;

        step_texel(s, d, level_size);

        vec2 limit = abs(s.xy * 2.0f - 1.0f);

        if(max(limit.x, limit.y) < 1.0f)
        {
            p = s.xy;
            float depth = hyperbolic_depth(
                texelFetch(linear_depth, ivec2(s.xy * level_size), level).r
            );

            if(s.z <= depth)
            {
                //level++;
                level_size = size >> level;
            }
            else
            {
                float t = (s.z - depth) / d.z;
                s -= d * t;
                level--;
                level_size = size >> level;
            }
        }
        else break;*/
    }
    return level == -1;
}
