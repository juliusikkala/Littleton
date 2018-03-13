struct shadow_map
{
    samplerCube map;
    float far_plane;

    float min_bias;
    float max_bias;
    float radius;
    int samples;
};

uniform sampler1D shadow_kernel;
uniform sampler2D shadow_noise;

float shadow_coef(
    in shadow_map sm,
    vec3 dir, // This must be in world space!
    float ndotd
){
    float map_depth = texture(sm.map, dir).x * sm.far_plane;
    float depth = length(dir);

    float bias = max(
        sm.max_bias * (1.0f - ndotd),
        sm.min_bias
    );

    return (depth - bias) <= map_depth ? 1.0f : 0.0f;
}


