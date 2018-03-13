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

uniform sampler1D shadow_kernel;
uniform sampler2D shadow_noise;

float shadow_coef(
    in shadow_map sm,
    vec4 light_space_pos,
    vec3 normal,
    vec3 light_dir
){
    vec3 pos = light_space_pos.xyz / light_space_pos.w;
    pos = pos * 0.5f + 0.5f;
    if(pos.z > 1.0f) return 1.0f;

    float shadow = 0.0f;
    float bias = max(
        sm.max_bias * (1.0f - dot(normal, light_dir)),
        sm.min_bias
    );

    ivec2 tex_size = textureSize(sm.map, 0);
    ivec2 noise_size = textureSize(shadow_noise, 0);
    vec2 texel = 1.0f/vec2(tex_size);

    ivec2 sample_pos = ivec2(fract(pos.xy * tex_size) * noise_size);
    vec2 cs = texelFetch(shadow_noise, sample_pos, 0).xy;
    mat2 rotation = mat2(cs.x, cs.y, -cs.y, cs.x);

    for(int i = 0; i < sm.samples; ++i)
    {
        vec2 sample_offset =
            rotation * texelFetch(shadow_kernel, i, 0).xy * sm.radius;

        shadow += dot(
            textureGather(sm.map, pos.xy + sample_offset * texel, pos.z - bias),
            vec4(0.25/sm.samples)
        );
    }

    return shadow;
}

