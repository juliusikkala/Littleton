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

vec4 get_color()
{
#ifdef MATERIAL_COLOR_CONSTANT
    return material.color;
#elif defined(MATERIAL_COLOR_TEXTURE)
    return texture(
        material.color,
#ifdef VERTEX_UV
        f_uv
#else
        vec2(0.0f)
#endif
    );
#else
    return vec4(1.0f, 0.0f, 0.0f, 1.0f);
#endif
}

void main(void)
{
    vec4 diffuse_color = get_color();
    vec4 color;
#ifdef VERTEX_NORMAL
    vec3 normal = normalize(f_normal);
#endif

#if defined(LIGHTING) && defined(VERTEX_NORMAL)
    color = vec4(0.0f,0.0f,0.0f,diffuse_color.a);

#if POINT_LIGHT_COUNT > 0
    for(int i = 0; i < POINT_LIGHT_COUNT; ++i)
    {
        float diffuse = point_light_diffuse(lights.point[i], f_position, normal);
        color.rgb += diffuse_color.rgb * diffuse;
    }
#endif
#if DIRECTIONAL_LIGHT_COUNT > 0
    for(int i = 0; i < DIRECTIONAL_LIGHT_COUNT; ++i)
    {
        float diffuse = directional_light_diffuse(lights.directional[i], normal);
        color.rgb += diffuse_color.rgb*diffuse;
    }
#endif
#if SPOTLIGHT_COUNT > 0
    for(int i = 0; i < SPOTLIGHT_COUNT; ++i)
    {
        float diffuse = spotlight_diffuse(lights.spot[i], f_position, normal);
        color.rgb += diffuse_color.rgb*diffuse;
    }
#endif
#else
    color = diffuse_color;
#endif
    if(color.a < 0.5f) discard;
    out_color = color;
}
