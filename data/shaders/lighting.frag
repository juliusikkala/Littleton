/* This shader is meant to be used with lighting.vert. */
#version 400 core

#include "light_types.glsl"
#include "deferred_input.glsl"
#include "shadow.glsl"

#ifdef POINT_LIGHT
uniform point_light light;
#elif defined(SPOTLIGHT)
uniform spotlight light;
#elif defined(DIRECTIONAL_LIGHT)
uniform directional_light light;
#endif

out vec4 out_color;
uniform shadow_map shadow;

void main(void)
{
    vec3 surface_color = get_albedo();
    vec3 normal = decode_normal();
    float roughness, metallic, f0;
    decode_material(roughness, metallic, f0);
    roughness = roughness * roughness;

    vec3 pos = decode_position();
    vec3 view_dir = normalize(-pos);

#ifdef POINT_LIGHT
    vec3 lighting = calc_point_light(
        light,
        pos,
        surface_color,
        view_dir,
        normal,
        roughness,
        f0,
        metallic
    );
#elif defined(SPOTLIGHT)
    vec3 lighting = calc_spotlight(
        light,
        pos,
        surface_color,
        view_dir,
        normal,
        roughness,
        f0,
        metallic
    );
#elif defined(DIRECTIONAL_LIGHT)
    vec3 lighting = calc_directional_light(
        light,
        surface_color,
        view_dir,
        normal,
        roughness,
        f0,
        metallic
    );

    if(light.shadow_map_index == 0)
    {
        lighting *= shadow_coef(
            shadow.map,
            shadow.view_to_light * vec4(pos, 1.0f),
            get_shadow_bias(
                normal,
                light.direction,
                shadow.min_bias,
                shadow.max_bias
            )
        );
    }
#endif

    out_color = vec4(lighting, 1.0f);
}
