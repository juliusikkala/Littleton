/* This shader is meant to be used with generic.vert. It is suitable for
 * forward rendering, and supports materials.
 */
#version 330 core
struct material_t
{
#ifdef MATERIAL_COLOR_CONSTANT
    vec4 color;
#endif
};

uniform material_t material;

#ifdef VERTEX_NORMAL
in vec3 f_normal;

#ifdef VERTEX_TANGENT
in vec3 f_tangent;
in vec3 f_bitangent;
#endif
#endif

#ifdef VERTEX_UV
in vec2 f_uv;
#endif

out vec4 out_color;

vec4 get_color()
{
#ifdef MATERIAL_COLOR_CONSTANT
    return material.color;
#else
    return vec4(1.0f, 0.0f, 0.0f, 1.0f);
#endif
}

void main(void)
{
    out_color = get_color();
}
