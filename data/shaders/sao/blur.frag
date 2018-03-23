#version 400 core

#define EDGE_SHARPNESS 10.0
#define STRIDE 3
#define RADIUS 4

const float gaussian[] = float[](
    0.153170, 0.144893, 0.122649, 0.092902, 0.062970
);

in vec2 uv;
uniform sampler2D in_ao;
uniform sampler2D in_depth;
uniform ivec2 step_size;
out float out_ao;

void main(void)
{
    ivec2 size = textureSize(in_ao, 0);
    ivec2 pixel_pos = ivec2(size * uv);

    float ao = texelFetch(in_ao, pixel_pos, 0).x;
    float depth = texelFetch(in_depth, pixel_pos, 0).x;

    float sum = ao;

    float total_weight = gaussian[0];
    sum *= total_weight;

    for(int r = -RADIUS; r <= RADIUS; ++r)
    {
        if(r != 0)
        {
            ivec2 sample_pos = pixel_pos + step_size * r * STRIDE;
            float sample_ao = texelFetch(in_ao, sample_pos, 0).x;
            float sample_depth = texelFetch(in_depth, sample_pos, 0).x;

            float weight = 0.3 + gaussian[abs(r)];

            weight *= max(
                0.0f,
                1.0f - EDGE_SHARPNESS * abs(sample_depth - depth)
            );

            sum += sample_ao * weight;
            total_weight += weight;
        }
    }

    out_ao = sum / total_weight;
}
