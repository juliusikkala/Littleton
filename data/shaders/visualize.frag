/* This shader is for visualizing the G-Buffer. */
#version 330 core
#include "deferred_input.glsl"

out vec4 out_color;

void main(void)
{
#ifdef SHOW_DEPTH
    out_color = vec4(vec3(get_depth()), 1.0f);
#elif defined(SHOW_POSITION)
    vec3 pos = decode_position();
    out_color = vec4(pos.x, pos.y, -pos.z, 1.0f);
#elif defined(SHOW_NORMAL)
    out_color = vec4(decode_normal()*0.5f+0.5f, 1.0f);
#elif defined(SHOW_COLOR)
    out_color = vec4(get_albedo(), 1.0f);
#elif defined(SHOW_ROUGHNESS)
    float roughness, metallic, ior;
    decode_material(roughness, metallic, ior);
    out_color = vec4(vec3(roughness), 1.0f);
#elif defined(SHOW_METALLIC)
    float roughness, metallic, ior;
    decode_material(roughness, metallic, ior);
    out_color = vec4(vec3(metallic), 1.0f);
#elif defined(SHOW_IOR)
    float roughness, metallic, ior;
    decode_material(roughness, metallic, ior);
    out_color = vec4(vec3((ior-1.0f)/4.0f), 1.0f);
#elif defined(SHOW_MATERIAL)
    float roughness, metallic, ior;
    decode_material(roughness, metallic, ior);
    out_color = vec4(roughness, metallic, (ior-1.0f)/4.0f, 1.0f);
#else
    out_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
#endif
}
