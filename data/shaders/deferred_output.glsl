#include "constants.glsl"
#include "generic_fragment_input.glsl"
#include "material.glsl"
#include "projection.glsl"

#ifdef COLOR_INDEX
layout(location=COLOR_INDEX) out vec3 out_color;
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

#ifdef INDIRECT_LIGHTING_INDEX
layout(location=INDIRECT_LIGHTING_INDEX) out vec4 out_indirect_lighting;
#endif

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
    vec3 emission, float roughness, float metallic, float f0
){
#ifdef COLOR_INDEX
    out_color = color;
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
