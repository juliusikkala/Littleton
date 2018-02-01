/* This shader assumes that the input vertices are (1,1), (-1,1), (1,-1) and
 * (-1, 1). The z-coordinates do not matter, they're discarded anyway.
 * It's whole point is to simply enable a related fragment shader to be applied
 * on every pixel. See vertex_buffer::create_fullscreen() for creating a
 * compatible vertex buffer.
 */
#version 330 core

layout(location = 0) in vec2 vertex;
layout(location = 3) in vec2 v_uv;
out vec2 uv;

void main(void)
{
    gl_Position = vec4(vertex, 0, 1.0f);
    uv = v_uv;
}
