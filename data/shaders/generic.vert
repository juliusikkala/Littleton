/* This is a generic vertex shader, passing the vertex data modified by mvp. */
#version 330 core

#include "generic_vertex_input.glsl"

uniform mat4 mvp;
uniform mat4 m;
uniform mat3 n_m;

void main(void)
{
    f_position = vec3(m * vec4(v_vertex, 1.0f));
    gl_Position = mvp * vec4(v_vertex, 1.0f);

#ifdef VERTEX_NORMAL
    f_normal = n_m * v_normal;

#ifdef VERTEX_TANGENT
    f_tangent = n_m * v_tangent.xyz;
    f_bitangent = n_m * (cross(v_normal, v_tangent.xyz) * v_tangent.w);
#endif
#endif

#ifdef VERTEX_UV
    f_uv = v_uv;
#endif
}

