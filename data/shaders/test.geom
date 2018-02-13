#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

#include "generic_geometry_input.glsl"

vec3 get_normal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

uniform mat3 n_m;
uniform mat4 mvp;

void main()
{
    vec3 normal = get_normal();

    for(int i = 0; i < 3; ++i)
    {
        g_out.position = g_in[i].position;
        gl_Position = gl_in[i].gl_Position;
#ifdef VERTEX_NORMAL
        g_out.normal = g_in[i].normal;
#ifdef VERTEX_TANGENT
        g_out.tangent = g_in[i].tangent;
        g_out.bitangent = g_in[i].bitangent;
#endif
#endif
#ifdef VERTEX_UV
        g_out.uv = g_in[i].uv;
#endif
        EmitVertex();
    }
    EndPrimitive();
}
