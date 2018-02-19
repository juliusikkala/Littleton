/* This is a generic vertex shader, passing the vertex data modified by mvp. */
#version 400 core

#include "generic_vertex_input.glsl"

uniform mat4 mvp;
uniform mat4 m;

#ifdef VERTEX_NORMAL
uniform mat3 n_m;
#endif

void main(void)
{
    v_out.position = vec3(m * vec4(v_vertex, 1.0f));
    gl_Position = mvp * vec4(v_vertex, 1.0f);

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

