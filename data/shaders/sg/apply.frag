#version 430 core

#include "generic_fragment_input.glsl"
#include "deferred_input.glsl"
#include "sg.glsl"

uniform mat4 inv_mv;
uniform int sg_lobe_count;
uniform sampler3D sg_amplitude[MAX_LOBE_COUNT];
uniform vec3 sg_axis[MAX_LOBE_COUNT];
uniform float sg_sharpness[MAX_LOBE_COUNT];
uniform float min_specular_roughness;
out vec4 color;

void main(void)
{
    vec2 uv = gl_FragCoord.xy / textureSize(in_depth, 0);
    vec3 pos = decode_position(uv);
    vec3 cube_coord = (inv_mv * vec4(pos, 1)).xyz;
    if(any(greaterThan(abs(cube_coord), vec3(1.0f)))) discard;

    float roughness;
    float metallic;
    float f0;
    decode_material(uv, roughness, metallic, f0);

    vec3 cube_normal = normalize((inv_mv * vec4(decode_normal(uv), 0)).xyz);
    vec3 cube_view = normalize((inv_mv * vec4(-pos, 0)).xyz);
    vec3 surface_color = decode_color(uv);

    vec3 irradiance = vec3(0);
    vec3 specular = vec3(0);

    // Irradiance
    for(int i = 0; i < sg_lobe_count; ++i)
    {
        sg_lobe lobe;
        lobe.amplitude = texture(sg_amplitude[i], cube_coord*0.5f+0.5f).rgb;
        lobe.axis = sg_axis[i];
        lobe.sharpness = sg_sharpness[i];

        irradiance += sg_approx_irradiance(lobe, cube_normal);

        if(roughness > min_specular_roughness)
        {
            specular += asg_approx_specular(
                lobe,
                surface_color,
                cube_view,
                cube_normal,
                roughness,
                f0,
                metallic
            );
        }
    }

    color = vec4(max(surface_color * irradiance + specular, vec3(0.0f)), 0);
}

