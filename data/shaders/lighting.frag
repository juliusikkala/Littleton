/* This shader is meant to be used with lighting.vert. */
#version 330 core

#include "light_types.glsl"
#include "deferred_input.glsl"

#ifdef POINT_LIGHT
uniform point_light light;
#elif defined(SPOTLIGHT)
uniform spotlight light;
#elif defined(DIRECTIONAL_LIGHT)
uniform directional_light light;
#endif

out vec4 out_color;

void main(void)
{
    vec3 diffuse_color = get_albedo();
    vec3 normal = decode_normal();
#if !defined(DIRECTIONAL_LIGHT)
    vec3 pos = decode_position();
#endif

#ifdef POINT_LIGHT
    vec3 diffuse = point_light_diffuse(light, pos, normal);
#elif defined(SPOTLIGHT)
    vec3 diffuse = spotlight_diffuse(light, pos, normal);
#elif defined(DIRECTIONAL_LIGHT)
    vec3 diffuse = directional_light_diffuse(light, normal);
#endif

    out_color =
        vec4(diffuse_color.rgb*diffuse, 1.0f);
}
