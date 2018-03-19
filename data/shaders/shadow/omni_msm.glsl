// Moment Shadow Mapping
#include "shadow/msm.glsl"
#include "constants.glsl"

struct shadow_map
{
    samplerCube map;
    float far_plane;
};

float shadow_coef(
    in shadow_map sm,
    vec3 dir, // This must be in world space!
    float ndotd
){
    float depth = (length(dir) / sm.far_plane) * 2.0f - 1.0f;

    mat4 q = mat4(
        -1.0f/3.0f, 0.0f, -0.75f, 0.0f,
        0.0f, 0.125f, 0.0f, -0.125f,
        SQRT3, 0.0f, 0.75f*SQRT3, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f
    );
    vec4 m = q * (texture(sm.map, dir) - vec4(0.5f, 0.0f, 0.5f, 0.0f));
    float alpha = 6e-5;
    vec4 b = mix(m, vec4(0.0f, 0.628f, 0.0f, 0.628f), alpha);

    return 1.0f - msm_shadow_intensity(b, depth);
}


