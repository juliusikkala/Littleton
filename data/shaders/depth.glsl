uniform sampler2D in_depth;
uniform vec3 clip_info;
uniform vec2 projection_info;

float get_depth(vec2 uv)
{
    return texture(in_depth, uv).x;
}

float linearize_depth(float depth)
{
    return -2.0f * clip_info.x / (depth * clip_info.y + clip_info.z);
}

float get_linear_depth(vec2 uv)
{
    float depth = texture(in_depth, uv).x * 2.0f - 1.0f;
    return linearize_depth(depth);
}

vec3 get_position(float linear_depth, vec2 uv)
{
    return vec3((0.5f-uv) * projection_info * linear_depth, linear_depth);
}

vec3 decode_position(vec2 uv)
{
    return get_position(get_linear_depth(uv), uv);
}

