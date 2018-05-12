/* This shader is meant to be used with lighting.vert. */
#version 400 core

#include "light_types.glsl"
#include "generic_fragment_input.glsl"
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

#ifdef AMBIENT
uniform vec3 ambient;
#endif

#ifdef SHADOW_MAPPING
uniform shadow_map shadow;
#endif
#ifdef OMNI_SHADOW_MAPPING
uniform mat4 inv_view;
#endif

void main(void)
{
    vec2 uv = gl_FragCoord.xy / textureSize(in_depth, 0);
    vec3 surface_color = decode_color(uv);
    vec3 normal = decode_normal(uv);
    float roughness, metallic, f0;
    decode_material(uv, roughness, metallic, f0);
    roughness = roughness * roughness;

    vec3 pos = decode_position(uv);
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

#ifdef OMNI_SHADOW_MAPPING
    vec3 dir = pos - light.position;
    lighting *= shadow_coef(
        shadow,
        vec3(inv_view * vec4(dir, 0)),
        dot(dir, normal)
    );
#endif
#ifdef PERSPECTIVE_SHADOW_MAPPING
    vec3 ldir = pos - light.position;
    float ldir_len = length(ldir);
    lighting *= shadow_coef(
        shadow, shadow.mvp * vec4(pos, 1.0), ldir_len,
        dot(normal, ldir/ldir_len)
    );
#endif
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
#ifdef OMNI_SHADOW_MAPPING
    vec3 dir = pos - light.position;
    lighting *= shadow_coef(
        shadow,
        vec3(inv_view * vec4(dir, 0)),
        dot(dir, normal)
    );
#endif
#ifdef PERSPECTIVE_SHADOW_MAPPING
    vec3 ldir = pos - light.position;
    float ldir_len = length(ldir);
    lighting *= shadow_coef(
        shadow, shadow.mvp * vec4(pos, 1.0), ldir_len,
        dot(normal, ldir/ldir_len)
    );
#endif

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

#ifdef DIRECTIONAL_SHADOW_MAPPING
    lighting *= shadow_coef(
        shadow,
        shadow.mvp * vec4(pos, 1.0),
        dot(normal, -light.direction)
    );
#endif
#endif

#ifdef AMBIENT
    vec3 lighting = surface_color * ambient;
#endif

#ifdef VISUALIZE
    lighting += vec3(0.1f);
#endif

    out_color = vec4(max(lighting, vec3(0)), 1.0f);
}
