#version 400 core

#define MAX_OFFSET 3
#define MAX_LEVEL 5
#define NUM_SPIRAL_TURNS 17

#include "constants.glsl"
#include "deferred_input.glsl"

uniform float radius;
uniform float proj_scale;
uniform int samples;
uniform float bias;
uniform float multiplier;

in vec2 uv;
out float out_ao;

void main(void)
{
    ivec2 depth_size = textureSize(in_depth, 0);
    ivec2 pixel_pos = ivec2(depth_size * uv);

    float depth = texelFetch(in_depth, pixel_pos, 0).x;

    vec3 pos = get_position(depth, uv);
#ifdef USE_NORMAL_TEXTURE
    vec3 normal = -decode_normal(uv);
#else
    vec3 normal = normalize(cross(dFdy(pos), dFdx(pos)));
#endif

    // Assign a "random" rotation angle for each pixel
    float base_angle =
        (3 * pixel_pos.x ^ pixel_pos.y + pixel_pos.x*pixel_pos.y) & 511;

    // Screen space radius in pixels
    float r = - proj_scale * radius / depth;

    // Calculate approximate occlusion
    float occlusion = 0;

    float radius2 = radius * radius;

    for(int i = 0; i < samples; ++i)
    {
        float alpha = float(i + 0.5f) / samples;
        float h = alpha * r;

        float angle = alpha * (NUM_SPIRAL_TURNS * 2.0f * PI) + base_angle;

        ivec2 orig = ivec2(h * vec2(cos(angle), sin(angle))) + pixel_pos;
        ivec2 sample_coord = clamp(
            orig,
            ivec2(0),
            depth_size-1
        );

        // Don't let outside pixels add to occlusion
        if(orig == sample_coord)
        {
            vec3 sample_pos;

            int level = clamp(findMSB(int(h)) - MAX_OFFSET, 0, MAX_LEVEL);
            sample_pos.z =
                texelFetch(in_depth, sample_coord.xy >> level, level).x;

            sample_pos.xy = vec2(sample_coord) + 0.5f;

            sample_pos = get_position(sample_pos.z, sample_pos.xy / depth_size);

            vec3 v = pos - sample_pos;

            float vv = dot(v, v);
            float vn = dot(v, normal);

            float f = max(radius2 - vv, 0.0f);
            occlusion += f * f * f * max((vn - bias) / vv, 0.0f);
        }
	}

    occlusion = max(0.0f, 1.0f - occlusion * multiplier);

    // Bilateral box filtering
	if(abs(dFdx(pos.z)) < 0.02f)
        occlusion -= dFdx(occlusion) * (float(pixel_pos.x & 1) - 0.5f);
    if(abs(dFdy(pos.z)) < 0.02f)
        occlusion -= dFdy(occlusion) * (float(pixel_pos.y & 1) - 0.5f);

    out_ao = occlusion;
}
