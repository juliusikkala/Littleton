#version 400 core

#include "deferred_input.glsl"

uniform sampler2D occlusion;

#ifndef USE_INDIRECT_LIGHTING
uniform vec3 ambient;
#endif

in vec2 uv;
out vec3 out_color;

void main(void)
{
#ifdef USE_INDIRECT_LIGHTING
    out_color = decode_indirect_lighting(uv) * texture(occlusion, uv).x;
#else
    vec3 surface_color = decode_color(uv);

    out_color = surface_color * ambient * texture(occlusion, uv).x;
#endif
}
