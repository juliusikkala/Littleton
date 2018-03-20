// PCF-filtered shadow mapping implementation

uniform sampler1D shadow_kernel;
uniform sampler2D shadow_noise;

float pcf(
    in sampler2DShadow map,
    int samples,
    vec2 uv,
    float depth,
    float bias,
    float radius
){
    float shadow = 0.0f;

    ivec2 tex_size = textureSize(map, 0);
    ivec2 noise_size = textureSize(shadow_noise, 0);
    vec2 texel = 1.0f/vec2(tex_size);

    ivec2 sample_pos = ivec2(fract(uv * tex_size) * noise_size);
    vec2 cs = texelFetch(shadow_noise, sample_pos, 0).xy;
    mat2 rotation = mat2(cs.x, cs.y, -cs.y, cs.x);

    for(int i = 0; i < samples; ++i)
    {
        vec2 sample_offset =
            rotation * texelFetch(shadow_kernel, i, 0).xy * radius;

        shadow += dot(
            textureGather(map, uv + sample_offset * texel, depth - bias),
            vec4(0.25/samples)
        );
    }

    return shadow;
}
