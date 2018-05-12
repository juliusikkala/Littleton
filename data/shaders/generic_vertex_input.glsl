#ifndef VERTEX_POSITION
#define VERTEX_POSITION 0
#endif

layout(location = VERTEX_POSITION) in vec3 v_vertex;

out VERTEX_OUT {
    vec3 position;
#if defined(DIRECTIONAL_SHADOW_MAPPING) || defined(PERSPECTIVE_SHADOW_MAPPING)
    vec4 light_space_pos;
#endif
#ifdef VERTEX_NORMAL
    vec3 normal;
#ifdef VERTEX_TANGENT
    vec3 tangent;
    vec3 bitangent;
#endif
#endif
#ifdef VERTEX_UV0
    vec2 uv;
#endif
} v_out;

#ifdef VERTEX_NORMAL
layout(location = VERTEX_NORMAL) in vec3 v_normal;

#ifdef VERTEX_TANGENT
layout(location = VERTEX_TANGENT) in vec4 v_tangent;
#endif
#endif

#ifdef VERTEX_UV0
layout(location = VERTEX_UV0) in vec2 v_uv;
#endif
