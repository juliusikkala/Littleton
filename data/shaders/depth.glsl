uniform sampler2D in_depth;
uniform sampler2D in_linear_depth;
uniform vec3 clip_info;

float get_depth(vec2 uv)
{
    return texture(in_depth, uv).x;
}

float linearize_depth(float depth)
{
    return -2.0f * clip_info.x / (depth * clip_info.y + clip_info.z);
}

float hyperbolic_depth(float linear_depth)
{
    return (-2.0f * clip_info.x - clip_info.z * linear_depth) /
           (linear_depth * clip_info.y);
}

float get_linear_depth(vec2 uv)
{
    float depth = texture(in_depth, uv).x * 2.0f - 1.0f;
    return linearize_depth(depth);
}
