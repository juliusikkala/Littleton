/* This is a generic vertex shader, passing the vertex data modified by mvp. */
#version 400 core

#include "generic_vertex_input.glsl"

uniform mat4 mvp;
uniform mat4 m;

#ifdef VERTEX_NORMAL
uniform mat3 n_m;
#endif

#if MAX_SHADOW_MAP_COUNT > 0
#include "shadow.glsl"
uniform int shadow_map_count;
uniform shadow_map shadows[MAX_SHADOW_MAP_COUNT];
#endif

void main(void)
{
    v_out.position = vec3(m * vec4(v_vertex, 1.0f));
    gl_Position = mvp * vec4(v_vertex, 1.0f);

#if MAX_SHADOW_MAP_COUNT > 0
    for(int i = 0; i < shadow_map_count; ++i)
    {
        v_out.shadow_pos[i] = shadows[i].mvp * vec4(v_vertex, 1.0f);
    }
#endif

#ifdef VERTEX_NORMAL
    v_out.normal = n_m * v_normal;

#ifdef VERTEX_TANGENT
    v_out.tangent = n_m * v_tangent.xyz;
    v_out.bitangent = n_m * (cross(v_normal, v_tangent.xyz) * v_tangent.w);
#endif
#endif

#ifdef VERTEX_UV
    v_out.uv = v_uv;
#endif
}

