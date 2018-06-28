#version 400 core
// This is simply 3*6 but GLSL doesn't allow computations in layout :/
#define CUBEMAP_MAX_VERTICES 18
#define CUBEMAP_LAYER_FACES 6

uniform int begin_layer_face;

layout (triangles) in;
layout (triangle_strip, max_vertices=CUBEMAP_MAX_VERTICES) out;

uniform mat4 face_ivps[CUBEMAP_LAYER_FACES];

layout(location = 0) in vec2 pos[];
layout(location = 0) out vec3 view_dir;

void main(void)
{
    for(int layer_face = 0; layer_face < CUBEMAP_LAYER_FACES; ++layer_face)
    {
        gl_Layer = begin_layer_face + layer_face;
        for(int i = 0; i < 3; ++i)
        {
            view_dir = (face_ivps[layer_face] * vec4(pos[i], -1.0f, 1.0f)).xyz;
            gl_Position = gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}

