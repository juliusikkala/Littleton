#version 430 core

#include "generic_fragment_input.glsl"
#include "deferred_input.glsl"

uniform mat4 inv_mv;
out vec4 color;

void main(void)
{
    vec2 uv = gl_FragCoord.xy / textureSize(in_depth, 0);
    vec3 pos = decode_position(uv);
    vec3 cube_coord = (inv_mv * vec4(pos, 1)).xyz;
    if(any(greaterThan(abs(cube_coord), vec3(1.0f)))) discard;
    //vec3 surface_color = texelFetch(in_color, ivec2(gl_FragCoord.xy), 0).rgb;
    vec3 surface_color = cube_coord;
    color = vec4(surface_color*vec3(0.001f),0);
}


