/* This is a generic vertex shader, passing the vertex data modified by mvp. */
#version 330 core

layout(location = VERTEX_POSITION) in vec3 v_vertex;

#ifdef VERTEX_NORMAL
layout(location = VERTEX_NORMAL) in vec3 v_normal;
out vec3 f_normal;

#ifdef VERTEX_TANGENT
layout(location = VERTEX_TANGENT) in vec4 v_tangent;
out vec3 f_tangent;
out vec3 f_bitangent;
#endif
#endif

#ifdef VERTEX_UV
layout(location = VERTEX_UV) in vec2 v_uv;
out vec2 f_uv;
#endif

uniform mat4 mvp;
uniform mat3 n_mv;

void main(void)
{
    gl_Position = mvp * vec4(v_vertex, 1.0f);

#ifdef VERTEX_NORMAL
    f_normal = n_mv * v_normal;

#ifdef VERTEX_TANGENT
    f_tangent = n_mv * v_tangent;
    f_bitangent = n_mv * (cross(v_normal, v_tangent.xyz) * v_tangent.w);
#endif
#endif

#ifdef VERTEX_UV
    f_uv = v_uv;
#endif
}

