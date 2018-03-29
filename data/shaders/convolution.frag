/* 1D convolution. For 3x3 2D convolution, see kernel.frag */
#version 400 core

uniform sampler2D src;

uniform float kernel[MAX_CONVOLUTION_SIZE];
uniform int convolution_size;
uniform ivec2 step_size;

out vec4 dst;


void main(void)
{
    vec4 sum = vec4(0.0f);
    float total = 0.0f;

    ivec2 off = -(step_size * convolution_size >> 1);

    for(int i = 0; i < convolution_size; ++i)
    {
        float weight = kernel[i];
        vec4 value = texelFetch(src, off + ivec2(gl_FragCoord.xy), 0);
        off += step_size;
        sum += weight * value;
        total += weight;
    }

    dst = sum / total;
}
