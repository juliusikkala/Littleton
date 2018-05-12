#version 400 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 face_vps[6];

#include "generic_geometry_input.glsl"

void main(void)
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for(int i = 0; i < 3; ++i)
        {
            g_out.position = g_in[i].position;
#ifdef VERTEX_NORMAL
            g_out.normal = g_in[i].normal;
#endif

#ifdef VERTEX_TANGENT
            g_out.tangent = g_in[i].tangent;
            g_out.bitangent = g_in[i].bitangent;
#endif
#ifdef VERTEX_UV0
            g_out.uv = g_in[i].uv;
#endif

            gl_Position = face_vps[face] * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}
