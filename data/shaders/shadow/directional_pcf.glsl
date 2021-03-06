// PCF-filtered shadow mapping implementation

struct shadow_map
{
    sampler2DShadow map;
    float min_bias;
    float max_bias;
    float radius;
    mat4 mvp;
    int samples;
};

#include "shadow/pcf.glsl"

float shadow_coef(
    in shadow_map sm,
    vec4 light_space_pos,
    float ndotl
){
    vec3 pos = light_space_pos.xyz / light_space_pos.w;
    pos = pos * 0.5f + 0.5f;
    if(abs(pos.z) > 1.0f) return 1.0f;

    float bias = max(
        sm.max_bias * (1.0f - ndotl),
        sm.min_bias
    );

    return pcf(sm.map, sm.samples, pos.xy, pos.z, bias, sm.radius);
}

