#version 400 core

uniform sampler2D prev;
in vec2 uv;

#if !defined(MAXIMUM) || !defined(MINIMUM)
out float output;
#else
out vec2 output;
#endif

void main(void)
{
#if defined(MINIMUM)
    vec4 minimums = textureGather(prev, uv, 0);
    float minimum =
        min(min(min(minimums.x, minimums.y), minimums.z), minimums.w);
#endif
#if defined(MAXIMUM)
    vec4 maximums = textureGather(prev, uv, 1);
    float maximum =
        max(max(max(maximums.x, maximums.y), maximums.z), maximums.w);
#endif

#if defined(MAXIMUM) && defined(MINIMUM)
    output = vec2(minimum, maximum);
#elif defined(MAXIMUM)
    output = maximum;
#elif defined(MINIMUM)
    output = minimum;
#endif
}
