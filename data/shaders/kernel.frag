/* This shader applies a kernel to the image. */
#version 400 core

uniform sampler2D in_color;
uniform mat3 kernel;
uniform vec2 pixel_offset;
in vec2 uv;
out vec4 out_color;

void main(void)
{
    vec4 c = vec4(0.0f);
    c += kernel[0][0] * texture(in_color, uv + vec2(-pixel_offset.x, pixel_offset.y));
    c += kernel[1][0] * texture(in_color, uv + vec2(0.0f, pixel_offset.y));
    c += kernel[2][0] * texture(in_color, uv + vec2(pixel_offset.x, pixel_offset.y));
    c += kernel[0][1] * texture(in_color, uv + vec2(-pixel_offset.x, 0.0f));
    c += kernel[1][1] * texture(in_color, uv + vec2(0.0f, 0.0f));
    c += kernel[2][1] * texture(in_color, uv + vec2(pixel_offset.x, 0.0f));
    c += kernel[0][2] * texture(in_color, uv + vec2(-pixel_offset.x, -pixel_offset.y));
    c += kernel[1][2] * texture(in_color, uv + vec2(0.0f, -pixel_offset.y));
    c += kernel[2][2] * texture(in_color, uv + vec2(pixel_offset.x, -pixel_offset.y));
    out_color = c;
}
