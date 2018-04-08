/* Simply copies the input texture color, no scaling or panning. Useful with
 * alpha blending. */
#version 400 core

uniform sampler2D tex;
out vec4 c;

void main(void)
{
    c = texelFetch(tex, ivec2(gl_FragCoord.xy), 0);
}
