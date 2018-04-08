#include "constants.glsl"
#include "generic_fragment_input.glsl"
#include "material.glsl"
#include "projection.glsl"

#ifdef COLOR_EMISSION_INDEX
layout(location=COLOR_EMISSION_INDEX) out vec4 out_color_emission;
#endif

#if defined(VERTEX_NORMAL) && defined(NORMAL_INDEX)
layout(location=NORMAL_INDEX) out vec2 out_normal;
#endif

#ifdef MATERIAL_INDEX
layout(location=MATERIAL_INDEX) out vec4 out_material;
#endif

#ifdef LINEAR_DEPTH_INDEX
layout(location=LINEAR_DEPTH_INDEX) out LINEAR_DEPTH_TYPE out_linear_depth;
#endif

#ifdef LIGHTING_INDEX
layout(location=LIGHTING_INDEX) out vec4 out_lighting;
#else
#undef OUTPUT_LIGHTING
#endif

vec4 encode_color_emission(vec3 color, float emission)
{
    return vec4(color, 1.0f/(emission + 1.0f));
}

vec2 encode_normal(vec3 normal)
{
    return project_lambert_azimuthal_equal_area(normal);
}

vec4 encode_material(float roughness, float metallic, float f0)
{
    return vec4(roughness, metallic, f0, 0.0f);
}

void write_gbuffer(
    vec3 view_pos, vec3 normal, vec3 color,
    float emission, float roughness, float metallic, float f0
){
#ifdef COLOR_EMISSION_INDEX
    out_color_emission = encode_color_emission(color, emission);
#endif
#if defined(VERTEX_NORMAL) && defined(NORMAL_INDEX)
    out_normal = encode_normal(normal);
#endif
#ifdef MATERIAL_INDEX
    out_material = encode_material(roughness, metallic, f0);
#endif
#ifdef LINEAR_DEPTH_INDEX
    out_linear_depth = LINEAR_DEPTH_TYPE(view_pos.z);
#endif
}
