/* This shader is meant to be used with generic.vert. It is suitable for
 * forward rendering, and supports materials.
 */
#version 330 core

#include "generic_fragment_input.glsl"
#include "material.glsl"
#include "light_types.glsl"

#ifdef LIGHTING
uniform Lights
{
#if POINT_LIGHT_COUNT > 0
    point_light point[POINT_LIGHT_COUNT];
#endif
#if DIRECTIONAL_LIGHT_COUNT > 0
    directional_light directional[DIRECTIONAL_LIGHT_COUNT];
#endif
#if SPOTLIGHT_COUNT > 0
    spotlight spot[SPOTLIGHT_COUNT];
#endif
} lights;
#endif

out vec4 out_color;

void main(void)
{
    vec4 surface_color = get_material_color();
    vec4 color;
#ifdef VERTEX_NORMAL
    vec3 normal = normalize(f_in.normal);
#if defined(MATERIAL_NORMAL_TEXTURE) && defined(VERTEX_TANGENT)
    mat3 tbn = mat3(normalize(f_in.tangent), normalize(f_in.bitangent), normal);
    vec3 ts_normal = get_material_normal();
    normal = normalize(tbn * ts_normal);
#endif
#endif

#if defined(LIGHTING) && defined(VERTEX_NORMAL)
    color = vec4(0.0f, 0.0f, 0.0f, surface_color.a);
    vec3 pos = f_in.position;
    vec3 view_dir = normalize(-f_in.position);
    float roughness = get_material_roughness();
    roughness = roughness * roughness;
    float metallic = get_material_metallic();
    float f0 = get_material_f0() / 2.0f;

#if POINT_LIGHT_COUNT > 0
    for(int i = 0; i < POINT_LIGHT_COUNT; ++i)
    {
        color.rgb += calc_point_light(
            lights.point[i],
            f_in.position,
            surface_color.rgb,
            view_dir,
            normal,
            roughness,
            f0,
            metallic
        );
    }
#endif

#if DIRECTIONAL_LIGHT_COUNT > 0
    for(int i = 0; i < DIRECTIONAL_LIGHT_COUNT; ++i)
    {
        color.rgb += calc_directional_light(
            lights.directional[i],
            surface_color.rgb,
            view_dir,
            normal,
            roughness,
            f0,
            metallic
        );
    }
#endif

#if SPOTLIGHT_COUNT > 0
    for(int i = 0; i < SPOTLIGHT_COUNT; ++i)
    {
        color.rgb += calc_spotlight(
            lights.spot[i],
            f_in.position,
            surface_color.rgb,
            view_dir,
            normal,
            roughness,
            f0,
            metallic
        );
    }
#endif

#else
    color = surface_color;
#endif
    if(color.a < 0.5f) discard;
    out_color = color;
}
