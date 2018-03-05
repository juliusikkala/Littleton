#version 400 core

in vec2 uv;
out vec4 color;

uniform sampler2D tex;
uniform int samples;

void main(void)
{
    ivec2 size = textureSize(tex, 0);
#ifdef VERTICAL
    vec2 step_size = vec2(0.0f, 1.0f / size.y);
#else
    vec2 step_size = vec2(1.0f / size.x, 0.0f);
#endif
    vec2 cur = -step_size * (samples - 1.0f)/2.0f;

    color = vec4(0.0f);

    for(int i = 0; i < samples; ++i)
    {
        cur += step_size;
        color += texture(tex, uv + cur);
    }

    color /= samples;
}
