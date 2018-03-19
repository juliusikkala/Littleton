struct shadow_map
{
    samplerCubeShadow map;
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
    float depth = length(dir);
    float bias = max(
        sm.max_bias * (1.0f - ndotd),
        sm.min_bias
    );

    float shadow = 0.0f;

    ivec2 tex_size = textureSize(sm.map, 0);
    ivec2 noise_size = textureSize(shadow_noise, 0);

    ivec2 sample_pos =
        ivec2(fract(vec2(dir.x+dir.z, dir.y+dir.z) * tex_size) * noise_size);
    vec3 random_vec = texelFetch(shadow_noise, sample_pos, 0).xyz;
    vec3 ndir = dir / depth;
    vec3 tangent = normalize(random_vec - ndir * dot(random_vec, ndir));
    mat2x3 tangent_space = mat2x3(tangent, cross(ndir, tangent));
    
    for(int i = 0; i < sm.samples; ++i)
    {
        vec3 sample_offset =
            tangent_space * texelFetch(shadow_kernel, i, 0).xy * sm.radius;

        shadow += dot(
            textureGather(
                sm.map,
                dir + sample_offset,
                depth/sm.far_plane - bias
            ),
            vec4(0.25f/sm.samples)
        );
    }

    return shadow;
}


