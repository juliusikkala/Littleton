struct material_t
{
#ifdef MATERIAL_COLOR_CONSTANT
    vec4 color;
#elif defined(MATERIAL_COLOR_TEXTURE)
    sampler2D color;
#endif

#ifdef MATERIAL_METALLIC_CONSTANT
    float metallic;
#elif defined(MATERIAL_METALLIC_TEXTURE)
    sampler2D metallic;
#endif

#ifdef MATERIAL_ROUGHNESS_CONSTANT
    float roughness;
#elif defined(MATERIAL_ROUGHNESS_TEXTURE)
    sampler2D roughness;
#endif

#ifdef MATERIAL_NORMAL_TEXTURE
    sampler2D normal;
#endif

    float f0;
};

uniform material_t material;

vec4 get_material_color()
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

float get_material_metallic()
{
#ifdef MATERIAL_METALLIC_CONSTANT
    return material.metallic;
#elif defined(MATERIAL_METALLIC_TEXTURE)
    return texture(
        material.metallic,
#ifdef VERTEX_UV
        f_uv
#else
        vec2(0.0f)
#endif
    ).x;
#else
    return 0.0f;
#endif
}

float get_material_roughness()
{
#ifdef MATERIAL_ROUGHNESS_CONSTANT
    return material.roughness;
#elif defined(MATERIAL_ROUGHNESS_TEXTURE)
    return texture(
        material.roughness,
#ifdef VERTEX_UV
        f_uv
#else
        vec2(0.0f)
#endif
    ).x;
#else
    return 0.0f;
#endif
}

#ifdef MATERIAL_NORMAL_TEXTURE
vec3 get_material_normal()
{
    return texture(
        material.normal,
#ifdef VERTEX_UV
        f_uv
#else
        vec2(0.0f)
#endif
    ).xyz*2.0f-1.0f;
}
#endif

float get_material_f0()
{
    return material.f0;
}
