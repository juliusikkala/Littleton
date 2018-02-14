uniform sampler2D in_depth;
uniform vec4 perspective_data;

float get_depth()
{
    return texture(in_depth, uv).x;
}

float get_linear_depth()
{
    float depth = texture(in_depth, uv).x * 2.0f - 1.0f;
    float n = perspective_data.z;
    float f = perspective_data.w;
    // Linearize depth
    depth = 2.0f * n * f / (n + f - depth * (f - n));
    return depth;
}

vec3 decode_position()
{
    float depth = get_linear_depth();
    return vec3((0.5f-uv)*perspective_data.xy*depth, depth);
}

