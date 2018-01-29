/* This shader is meant to be used with generic.vert. It is suitable for
 * forward rendering, and supports materials.
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

#if defined(LIGHT_COUNT) && LIGHT_COUNT >= 1
uniform Lights
{
    vec3 colors[LIGHT_COUNT];
    vec3 positions[LIGHT_COUNT];
} lights;
#endif

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

out vec4 out_color;

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

void main(void)
{
    vec4 diffuse_color = get_color();
    vec4 color;
#ifdef VERTEX_NORMAL
    vec3 normal = normalize(f_normal);
#endif

#if defined(LIGHT_COUNT) && defined(VERTEX_NORMAL)
    color = vec4(0.0f,0.0f,0.0f,1.0f);

#if LIGHT_COUNT > 0
    for(int i = 0; i < LIGHT_COUNT; ++i)
    {
        vec3 dir = lights.positions[i] - f_position;
        float dist = length(dir);
        dir/=dist;
        float ctheta = clamp(dot(normal, dir), 0, 1);
        color.rgb += lights.colors[i]*diffuse_color.rgb*ctheta/(dist*dist);
    }
#endif
#else
    color = diffuse_color;
#endif
    out_color = color;
}
