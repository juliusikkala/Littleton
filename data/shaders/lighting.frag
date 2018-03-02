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

#ifdef SHADOW_IMPLEMENTATION
uniform shadow_map shadow;
#endif

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

#ifdef SHADOW_IMPLEMENTATION
    shadow_vertex_data data;
    calculate_shadow_vertex_data(shadow, data, pos);

    lighting *= shadow_coef(
        shadow,
        data,
        normal,
        -light.direction
    );
#endif
#endif

    out_color = vec4(lighting, 1.0f);
}
