#include "projection.glsl"

uniform int ray_max_level;
uniform int ray_max_steps;
uniform float thickness;

#undef RAY_MAX_LEVEL
#define RAY_MAX_LEVEL 6
#define RAY_MIN_LEVEL 0

bool cast_ray(
    in sampler2D linear_depth,
    vec2 initial_uv,
    vec3 origin,
    vec3 dir,
    out vec2 uv
){
    int level = RAY_MAX_LEVEL;
    int steps = ray_max_steps;

    float level_distance = 2.0f;

    ivec2 size = textureSize(linear_depth, 0);

    vec2 o = initial_uv;
    vec2 d = projected_ray_direction(origin, dir);
    d /= max(abs(d.x), abs(d.y));
    ivec2 id = ivec2(sign(d));

    float prev_depth = origin.z;
    vec2 prev_o = o;

    ivec2 p = ivec2(o*textureSize(linear_depth, level));
    while(steps-- != 0 && 0.0f <= o.x && o.x <= 1.0f && 0.0f <= o.y && o.y <= 1.0f)
    {
        ivec2 level_size = textureSize(linear_depth, level);
        vec2 sample_depth = texelFetch(linear_depth, p, level).xy;

        vec2 s = o * level_size - vec2(p) - 0.5f;
        vec2 t2 = (0.5f*sign(d) - s)/(d*level_size);
        ivec2 d_step = id;
        float t = t2.y;

        if(t2.x < t2.y)
        {
            d_step.y = 0;
            t = t2.x;
        }
        else if(t2.y < t2.x) d_step.x = 0;

        p += d_step;
        prev_o = o;
        o += t * d;
        level_distance -= 1.0f;

        float next_depth =
            origin.z + calculate_ray_length(origin, dir, o)*dir.z;

        float min_depth = min(prev_depth, next_depth) + 0.01f;
        float max_depth = max(prev_depth, next_depth) + 0.01f;

        if(
            min_depth > sample_depth.y ||
            max_depth + thickness < sample_depth.x
        ){// Miss
            prev_depth = next_depth;
            if(level_distance <= 0 && level != RAY_MAX_LEVEL)
            {// Switch to upper mip level
                level++;
                level_distance = sqrt(8.0f);
                p = ivec2(o*textureSize(linear_depth, level));
            }
        }
        else if(level == RAY_MIN_LEVEL)
        {// Hit!
            uv = vec2(p + 0.5f) / vec2(level_size);
            return true;
        }
        else
        {// Possibly hit on a lower mip level.
            level--;
            level_distance = sqrt(8.0f);
            o = prev_o;
            p = ivec2(o*textureSize(linear_depth, level));
        }
    }
    return false;
}
