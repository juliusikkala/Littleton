struct material_t
{
    vec4 color_factor;
#ifdef MATERIAL_COLOR_TEXTURE
    sampler2D color;
#endif

    float metallic_factor;
    float roughness_factor;
#ifdef MATERIAL_METALLIC_ROUGHNESS_TEXTURE
    sampler2D metallic_roughness;
#endif

#ifdef MATERIAL_NORMAL_TEXTURE
    sampler2D normal;
#endif

    float f0;

    vec3 emission_factor;
#ifdef MATERIAL_EMISSION_TEXTURE
    sampler2D emission;
#endif
};

uniform material_t material;

vec4 get_material_color()
{
#ifdef MATERIAL_COLOR_TEXTURE
    return texture(
        material.color,
#ifdef VERTEX_UV0
        f_in.uv
#else
        vec2(0.0f)
#endif
    ) * material.color_factor;
#else
    return material.color_factor;
#endif
}

vec2 get_material_metallic_roughness()
{
#ifdef MATERIAL_METALLIC_ROUGHNESS_TEXTURE
    return texture(
        material.metallic_roughness,
#ifdef VERTEX_UV0
        f_in.uv
#else
        vec2(0.0f)
#endif
    ).bg * vec2(material.metallic_factor, material.roughness_factor);
#else
    return vec2(material.metallic_factor, material.roughness_factor);
#endif
}

#ifdef MATERIAL_NORMAL_TEXTURE
vec3 get_material_normal()
{
    return normalize(texture(
        material.normal,
#ifdef VERTEX_UV0
        f_in.uv
#else
        vec2(0.0f)
#endif
    ).xyz * 2.0f - 1.0f);
}
#endif

vec3 get_material_emission()
{
#ifdef MATERIAL_EMISSION_TEXTURE
    return texture(
        material.emission,
#ifdef VERTEX_UV0
        f_in.uv
#else
        vec2(0.0f)
#endif
    ).xyz * material.emission_factor;
#else
    return material.emission_factor;
#endif
}

float get_material_f0()
{
    return material.f0;
}
