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

struct shadow_vertex_data
{
    vec4 light_space_pos;
};

void calculate_shadow_vertex_data(
    in shadow_map sm,
    out shadow_vertex_data data,
    vec3 vertex_pos
){
    data.light_space_pos = sm.mvp * vec4(vertex_pos, 1.0f);
}

#if SHADOW_MAP_KERNEL_SIZE > 0
uniform vec2 shadow_kernel[SHADOW_MAP_KERNEL_SIZE];
uniform sampler2D shadow_noise;
#endif

float shadow_coef(
    in shadow_map sm,
    in shadow_vertex_data data,
    vec3 normal,
    vec3 light_dir
){
    vec3 pos = data.light_space_pos.xyz / data.light_space_pos.w;
    pos = pos * 0.5f + 0.5f;
    if(pos.z > 1.0f) return 1.0f;

    float shadow = 0.0f;
    float bias = max(
        sm.max_bias * (1.0f - dot(normal, light_dir)),
        sm.min_bias
    );

#if SHADOW_MAP_KERNEL_SIZE > 0
    ivec2 tex_size = textureSize(sm.map, 0);
    ivec2 noise_size = textureSize(shadow_noise, 0);
    vec2 texel = 1.0f/vec2(tex_size);

    ivec2 sample_pos = ivec2(
        fract(pos.xy * tex_size) * noise_size
    );
    vec2 cs = texelFetch(shadow_noise, sample_pos, 0).xy;
    mat2 rotation = mat2(cs.x, cs.y, -cs.y, cs.x);

    for(int i = 0; i < sm.samples; ++i)
    {
        vec2 sample_offset = rotation * shadow_kernel[i] * sm.radius;
        shadow += dot(textureGather(
            sm.map,
            pos.xy + sample_offset * texel,
            pos.z - bias
        ), vec4(0.25/sm.samples));
    }

#else
    shadow = texture(sm.map, vec3(pos.xy, pos.z - bias));
#endif

    return shadow;
}

