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

struct point_light
{
    vec3 color;
    vec3 position;
};

struct directional_light
{
    vec3 color;
    vec3 direction;
};

struct spotlight
{
    vec3 color;
    vec3 position;
    vec3 direction;
    float cutoff;
    float exponent;
};

#ifdef LIGHTING
uniform Lights
{
#if POINT_LIGHT_COUNT > 0
    point_light point[POINT_LIGHT_COUNT];
#endif
#if DIRECTIONAL_LIGHT_COUNT > 0
    directional_light directional[DIRECTIONAL_LIGHT_COUNT];
#endif
#if SPOTLIGHT_COUNT > 0
    spotlight spot[SPOTLIGHT_COUNT];
#endif
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

#if defined(LIGHTING) && defined(VERTEX_NORMAL)
    color = vec4(0.0f,0.0f,0.0f,diffuse_color.a);

#if POINT_LIGHT_COUNT > 0
    for(int i = 0; i < POINT_LIGHT_COUNT; ++i)
    {
        point_light l = lights.point[i];
        vec3 dir = l.position - f_position;
        float dist = length(dir);
        dir/=dist;
        float ctheta = clamp(dot(normal, dir), 0, 1);
        color.rgb += l.color*diffuse_color.rgb*ctheta/(dist*dist);
    }
#endif
#if DIRECTIONAL_LIGHT_COUNT > 0
    for(int i = 0; i < DIRECTIONAL_LIGHT_COUNT; ++i)
    {
        directional_light l = lights.directional[i];
        vec3 dir = l.direction;
        float ctheta = clamp(dot(normal, -dir), 0, 1);
        color.rgb += l.color*diffuse_color.rgb*ctheta;
    }
#endif
#if SPOTLIGHT_COUNT > 0
    for(int i = 0; i < SPOTLIGHT_COUNT; ++i)
    {
        spotlight l = lights.spot[i];
        vec3 dir = l.position - f_position;
        float dist = length(dir);
        dir/=dist;
        float cutoff = dot(dir, -l.direction);
        cutoff = cutoff > l.cutoff ? 1-pow(1-(cutoff-l.cutoff)/(1-l.cutoff), l.exponent) : 0;

        float ctheta = clamp(dot(normal, dir), 0, 1)*cutoff;
        color.rgb += l.color*diffuse_color.rgb*ctheta/(dist*dist);
    }
#endif
#else
    color = diffuse_color;
#endif
    if(color.a < 0.5f) discard;
    out_color = color;
}
