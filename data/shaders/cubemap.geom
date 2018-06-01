#version 400 core
#ifndef CUBEMAP_MAX_VERTICES
// This is simply 3*6 but GLSL doesn't allow computations in layout :/
#define CUBEMAP_MAX_VERTICES 18
#endif

#define CUBEMAP_LAYER_FACES (CUBEMAP_MAX_VERTICES/3)

layout (triangles) in;
layout (triangle_strip, max_vertices=CUBEMAP_MAX_VERTICES) out;

uniform mat4 face_vps[CUBEMAP_LAYER_FACES];

#include "generic_geometry_input.glsl"

void main(void)
{
    for(int layer_face = 0; layer_face < CUBEMAP_LAYER_FACES; ++layer_face)
    {
        gl_Layer = layer_face;
        for(int i = 0; i < 3; ++i)
        {
            g_out.position = g_in[i].position;
#if defined(DIRECTIONAL_SHADOW_MAPPING) || defined(PERSPECTIVE_SHADOW_MAPPING)
            g_out.light_space_pos = g_in[i].light_space_pos;
#endif
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

            gl_Position = face_vps[layer_face] * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}
