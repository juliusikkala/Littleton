#version 400 core

#include "deferred_input.glsl"

uniform float radius;
uniform float bias;
uniform int samples;
uniform sampler2D noise;
uniform sampler1D kernel;
uniform mat4 proj;

out float output;

void main(void)
{
    vec3 normal = decode_normal(uv);
    vec3 pos = decode_position(uv);
    vec2 noise_size = textureSize(noise, 0);
    vec3 random_vec = texture(noise, gl_FragCoord.xy/noise_size).xyz;
    vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
    mat4 rotation = mat4(mat3(tangent, cross(normal, tangent), normal));
    mat4 scaling = mat4(radius); scaling[3][3] = 1.0f;
    mat4 translation = mat4(1.0f); translation[3] = vec4(pos, 1.0f);

    mat4 mvp = proj * translation * rotation * scaling;

    float occlusion = 0.0f;

    for(int i = 0; i < samples; ++i)
    {
        vec4 o = mvp * vec4(texelFetch(kernel, i, 0).xyz, 1.0f);
        o.xyz /= o.w;
        o.xy = 0.5f * o.xy + 0.5f;
        
        float sample_depth = get_linear_depth(o.xy);
        float offset_depth = linearize_depth(o.z, o.xy);

        float range_check = smoothstep(
            0.0, 1.0, radius/abs(offset_depth - sample_depth)
        );
        occlusion += sample_depth >= offset_depth + bias ? range_check : 0.0f;
    }
    output = 1.0f - occlusion / samples;
}
