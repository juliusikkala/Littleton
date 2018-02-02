/* This shader is meant to be used with generic.vert. It is suitable for
 * deferred rendering geometry pass, and supports materials.
 */
#version 330 core
struct material_t
{
#ifdef MATERIAL_COLOR_CONSTANT
    vec4 color;
#elif defined(MATERIAL_COLOR_TEXTURE)
    sampler2D color;
#endif
};

uniform material_t material;

in vec3 f_position;

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

layout(location=0) out vec4 color_emission;

#ifdef VERTEX_NORMAL
layout(location=1) out vec2 normal;
#endif

vec4 get_color()
{
#ifdef MATERIAL_COLOR_CONSTANT
    return material.color;
#elif defined(MATERIAL_COLOR_TEXTURE)
    return texture(
        material.color,
#ifdef VERTEX_UV
        f_uv
#else
        vec2(0.0f)
#endif
    );
#else
    return vec4(1.0f, 0.0f, 0.0f, 1.0f);
#endif
}

vec2 encode_normal(vec3 normal)
{
    vec3 n = normalize(normal);
    return inversesqrt(4.0f+4.0f*n.z)*n.xy;
}

vec3 decode_normal(vec2 n)
{
    vec2 n2 = n*2.0f;
    float d = dot(n2, n2);
    float f = sqrt(2 - d);
    return vec3(f*n2, 1 - d);
}

void main(void)
{
    vec4 color = get_color();

    if(color.a < 0.5f) discard;
    color_emission = color;

#ifdef VERTEX_NORMAL
    normal = encode_normal(f_normal);
#endif
}

