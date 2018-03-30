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
#ifdef OMNI_SHADOW_MAPPING
uniform mat4 inv_view;
#endif

#ifdef OUTPUT_GEOMETRY
#include "deferred_output.glsl"
#endif

uniform vec3 ambient;

#ifdef OUTPUT_LIGHTING
#ifdef OUTPUT_GEOMETRY
layout(location=3) out vec4 out_lighting;
#else
out vec4 out_lighting;
#endif
#endif

void main(void)
{
    vec4 surface_color = get_material_color();
    vec4 color = surface_color;
#ifdef MAX_ALPHA
    if(surface_color.a >= MAX_ALPHA) discard;
#endif
#ifdef MIN_ALPHA
    if(surface_color.a < MIN_ALPHA) discard;
#endif

#ifdef VERTEX_NORMAL
    vec3 normal = normalize(f_in.normal);
#if defined(MATERIAL_NORMAL_TEXTURE) && defined(VERTEX_TANGENT)
    mat3 tbn = mat3(normalize(f_in.tangent), normalize(f_in.bitangent), normal);
    vec3 ts_normal = get_material_normal();
    normal = normalize(tbn * ts_normal);
#endif
    color = vec4(0.0f, 0.0f, 0.0f, surface_color.a);
    vec3 pos = f_in.position;
    vec3 view_dir = normalize(-f_in.position);
    float roughness = get_material_roughness();
    float emission = get_material_emission();
    float metallic = get_material_metallic();
    float f0 = get_material_f0();

#ifdef OUTPUT_GEOMETRY
    out_color_emission = encode_color_emission(surface_color.rgb, emission);
    out_normal = encode_normal(normal);
    out_material = encode_material(roughness, metallic, f0);
#endif

    roughness = roughness * roughness;
    f0 /= 2.0f;

#ifdef OUTPUT_LIGHTING
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
    color.rgb *= shadow_coef(
        shadow,
        vec3(inv_view * vec4(dir, 0)),
        dot(dir, normal)
    );
#endif
#ifdef PERSPECTIVE_SHADOW_MAPPING
    vec3 ldir = pos - light.position;
    float ldir_len = length(ldir);
    color.rgb *= shadow_coef(
        shadow, f_in.light_space_pos, ldir_len, dot(normal, ldir/ldir_len)
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
    color.rgb *= shadow_coef(
        shadow,
        vec3(inv_view * vec4(dir, 0)),
        dot(dir, normal)
    );
#endif
#ifdef PERSPECTIVE_SHADOW_MAPPING
    vec3 ldir = pos - light.position;
    float ldir_len = length(ldir);
    color.rgb *= shadow_coef(
        shadow, f_in.light_space_pos, ldir_len, dot(normal, ldir/ldir_len)
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
        dot(normal, -light.direction)
    );
#endif
#endif // Directional light
#endif // Multiple lights
#endif // Output lighting
#endif // Vertex normal

#ifdef OUTPUT_LIGHTING
#ifdef APPLY_AMBIENT
    color.rgb += surface_color.rgb * ambient;
#endif
#ifdef APPLY_EMISSION
    color.rgb += surface_color.rgb * emission;
#endif

    out_lighting = color;
#endif
}
