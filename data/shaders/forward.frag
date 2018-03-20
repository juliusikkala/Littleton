/* This shader is meant to be used with generic.vert. It is suitable for
 * forward rendering and supports materials.
 */
#version 400 core

#include "generic_fragment_input.glsl"
#include "material.glsl"
#include "light_types.glsl"
#include "shadow.glsl"

#ifdef MULTIPLE_LIGHTS
uniform Lights
{
    int point_light_count;
#if MAX_POINT_LIGHT_COUNT > 0
    point_light point[MAX_POINT_LIGHT_COUNT];
#endif
    int directional_light_count;
#if MAX_DIRECTIONAL_LIGHT_COUNT > 0
    directional_light directional[MAX_DIRECTIONAL_LIGHT_COUNT];
#endif
    int spotlight_count;
#if MAX_SPOTLIGHT_COUNT > 0
    spotlight spot[MAX_SPOTLIGHT_COUNT];
#endif
} lights;

#elif defined(SINGLE_LIGHT)

#ifdef POINT_LIGHT
uniform point_light light;
#elif defined(SPOTLIGHT)
uniform spotlight light;
#elif defined(DIRECTIONAL_LIGHT)
uniform directional_light light;
#endif

#endif

#ifdef SHADOW_MAPPING
uniform shadow_map shadow;
#endif

uniform vec3 camera_pos;

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
    color = vec4(0.0f, 0.0f, 0.0f, surface_color.a);
    vec3 pos = f_in.position;
    vec3 view_dir = normalize(camera_pos-f_in.position);
    float roughness = get_material_roughness();
    roughness = roughness * roughness;
    float metallic = get_material_metallic();
    float f0 = get_material_f0() / 2.0f;

#if defined(MULTIPLE_LIGHTS)
#if MAX_POINT_LIGHT_COUNT > 0
    for(int i = 0; i < lights.point_light_count; ++i)
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

#if MAX_DIRECTIONAL_LIGHT_COUNT > 0
    for(int i = 0; i < lights.directional_light_count; ++i)
    {
        directional_light l = lights.directional[i];
        vec3 c = calc_directional_light(
            l,
            surface_color.rgb,
            view_dir,
            normal,
            roughness,
            f0,
            metallic
        );

        color.rgb += c;
    }
#endif

#if MAX_SPOTLIGHT_COUNT > 0
    for(int i = 0; i < lights.spotlight_count; ++i)
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

#elif defined(SINGLE_LIGHT)
#ifdef POINT_LIGHT
    color.rgb = calc_point_light(
        light,
        f_in.position,
        surface_color.rgb,
        view_dir,
        normal,
        roughness,
        f0,
        metallic
    );

#ifdef OMNI_SHADOW_MAPPING
    vec3 dir = pos - light.position;
    color.rgb *= shadow_coef(shadow, dir, dot(dir, normal));
#endif
#ifdef PERSPECTIVE_SHADOW_MAPPING
    color.rgb *= shadow_coef(
        shadow, f_in.light_space_pos, normal, length(pos - light.position)
    );
#endif

#elif defined(SPOTLIGHT)
    color.rgb = calc_spotlight(
        light,
        f_in.position,
        surface_color.rgb,
        view_dir,
        normal,
        roughness,
        f0,
        metallic
    );
#ifdef OMNI_SHADOW_MAPPING
    vec3 dir = pos - light.position;
    color.rgb *= shadow_coef(shadow, dir, dot(dir, normal));
#endif
#ifdef PERSPECTIVE_SHADOW_MAPPING
    color.rgb *= shadow_coef(
        shadow, f_in.light_space_pos, normal, length(pos - light.position)
    );
#endif

#elif defined(DIRECTIONAL_LIGHT)
    color.rgb = calc_directional_light(
        light,
        surface_color.rgb,
        view_dir,
        normal,
        roughness,
        f0,
        metallic
    );

#ifdef DIRECTIONAL_SHADOW_MAPPING
    color.rgb *= shadow_coef(
        shadow,
        f_in.light_space_pos,
        normal,
        -light.direction
    );
#endif
#endif

#else
    color = surface_color;
#endif
#else
    color = surface_color;
#endif

    if(color.a < 0.5f) discard;
    out_color = color;
}
