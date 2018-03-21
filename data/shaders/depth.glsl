uniform sampler2D in_depth;
uniform vec4 perspective_data;

float get_depth(vec2 uv)
{
    return texture(in_depth, uv).x;
}

float linearize_depth(float depth, vec2 uv)
{
    float n = perspective_data.z;
    float f = perspective_data.w;

    // Linearize depth
    return 2.0f * n * f / (n + f - depth * (f - n));
}

float get_linear_depth(vec2 uv)
{
    float depth = texture(in_depth, uv).x * 2.0f - 1.0f;
    return linearize_depth(depth, uv);
}

vec3 decode_position(vec2 uv)
{
    float depth = get_linear_depth(uv);
    return vec3((0.5f-uv)*perspective_data.xy*depth, depth);
}

