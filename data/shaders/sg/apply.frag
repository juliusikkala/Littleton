#version 430 core

#include "generic_fragment_input.glsl"
#include "deferred_input.glsl"
#include "sg.glsl"

uniform mat4 inv_mv;
uniform int sg_lobe_count;
uniform sampler3D sg_amplitude[MAX_LOBE_COUNT];
uniform vec3 sg_axis[MAX_LOBE_COUNT];
uniform float sg_sharpness[MAX_LOBE_COUNT];
out vec4 color;

void main(void)
{
    vec2 uv = gl_FragCoord.xy / textureSize(in_depth, 0);
    vec3 pos = decode_position(uv);
    vec3 cube_coord = (inv_mv * vec4(pos, 1)).xyz;
    if(any(greaterThan(abs(cube_coord), vec3(1.0f)))) discard;

    vec3 cube_normal = (inv_mv * vec4(decode_normal(uv), 0)).xyz;

    vec3 env_light = vec3(0);
    for(int i = 0; i < sg_lobe_count; ++i)
    {
        sg_lobe lobe;
        lobe.amplitude = texture(sg_amplitude[i], cube_coord).rgb;
        lobe.axis = sg_axis[i];
        lobe.sharpness = sg_sharpness[i];

        if(lobe.sharpness <= 0.0f) env_light += lobe.amplitude;
        else env_light += sg_approx_irradiance(lobe, cube_normal);
    }

    color = vec4(decode_color(uv) * env_light, 0);
}

