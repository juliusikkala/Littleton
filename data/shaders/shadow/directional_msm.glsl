// Moment Shadow Mapping
#include "shadow/msm.glsl"
#include "constants.glsl"

struct shadow_map
{
    sampler2D map;
    mat4 mvp;
};

float shadow_coef(
    in shadow_map sm,
    vec4 light_space_pos,
    vec3 normal,
    vec3 light_dir
){
    vec3 pos = light_space_pos.xyz / light_space_pos.w;
    if(abs(pos.z) >= 1.0f || abs(pos.x) >= 1.0f || abs(pos.y) >= 1.0f)
        return 1.0f;

    pos.xy = pos.xy * 0.5f + 0.5;

    mat4 q = mat4(
        -1.0f/3.0f, 0.0f, -0.75f, 0.0f,
        0.0f, 0.125f, 0.0f, -0.125f,
        SQRT3, 0.0f, 0.75f*SQRT3, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f
    );
    vec4 m = q * (texture(sm.map, pos.xy) - vec4(0.5f, 0.0f, 0.5f, 0.0f));
    float alpha = 6e-5;
    vec4 b = mix(m, vec4(0.0f, 0.628f, 0.0f, 0.628f), alpha);

    return 1.0f - msm_shadow_intensity(b, pos.z);
}

