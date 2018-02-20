struct shadow_map
{
    sampler2DShadow map;
    mat4 mvp;
    float min_bias;
    float max_bias;
};

float get_shadow_bias(
    vec3 normal,
    vec3 light_dir,
    float min_bias,
    float max_bias
){
    return max(max_bias * (1.0f - dot(normal, light_dir)), min_bias);
}

#if SHADOW_SAMPLE_COUNT > 0
uniform vec2 shadow_kernel[SHADOW_SAMPLE_COUNT];
uniform sampler2D shadow_noise;
#endif

float shadow_coef(
    in sampler2DShadow shadow_map,
    vec4 light_space_pos,
    float bias
){
    vec3 pos = light_space_pos.xyz / light_space_pos.w;
    pos = pos * 0.5f + 0.5f;
    if(pos.z > 1.0f) return 1.0f;

    float shadow = 0.0f;

#if SHADOW_SAMPLE_COUNT > 0
    ivec2 tex_size = textureSize(shadow_map, 0);
    ivec2 noise_size = textureSize(shadow_noise, 0);
    vec2 texel = 1.0f/vec2(tex_size);

    ivec2 sample_pos = ivec2(
        fract(pos.xy * tex_size) * noise_size
    );
    vec2 cs = texelFetch(shadow_noise, sample_pos, 0).xy;
    mat2 rotation = mat2(cs.x, cs.y, -cs.y, cs.x);

    for(int i = 0; i < SHADOW_SAMPLE_COUNT; ++i)
    {
        vec2 sample_offset = rotation * shadow_kernel[i];
        shadow += texture(
            shadow_map,
            vec3(pos.xy + sample_offset * texel, pos.z-bias)
        );
    }
    shadow /= SHADOW_SAMPLE_COUNT;
#else
    shadow = texture(shadow_map, vec3(pos.xy, pos.z - bias));
#endif

    return shadow;
}
