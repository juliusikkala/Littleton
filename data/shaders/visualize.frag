/* This shader is for visualizing the G-Buffer. */
#version 400 core
#include "deferred_input.glsl"

out vec4 out_color;

void main(void)
{
#ifdef SHOW_DEPTH
    out_color = vec4(vec3(get_depth()), 1.0f);
#elif defined(SHOW_POSITION)
    vec3 pos = decode_position(uv);
    out_color = vec4(pos.x, pos.y, -pos.z, 1.0f);
#elif defined(SHOW_NORMAL)
    out_color = vec4(decode_normal(uv)*0.5f+0.5f, 1.0f);
#elif defined(SHOW_COLOR)
    out_color = vec4(decode_color_emission(uv).rgb, 1.0f);
#elif defined(SHOW_ROUGHNESS)
    float roughness, metallic, f0;
    decode_material(uv, roughness, metallic, f0);
    out_color = vec4(vec3(roughness), 1.0f);
#elif defined(SHOW_METALLIC)
    float roughness, metallic, f0;
    decode_material(uv, roughness, metallic, f0);
    out_color = vec4(vec3(metallic), 1.0f);
#elif defined(SHOW_IOR)
    float roughness, metallic, f0;
    decode_material(uv, roughness, metallic, f0);
    out_color = vec4(vec3(f0*2.0f), 1.0f);
#elif defined(SHOW_MATERIAL)
    float roughness, metallic, f0;
    decode_material(uv, roughness, metallic, f0);
    out_color = vec4(roughness, metallic, f0 * 2.0f, 1.0f);
#else
    out_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
#endif
}
