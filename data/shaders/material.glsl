struct material_t
{
    vec4 color;
    float metallic;
    float roughness;
    vec3 normal;
    float f0;
    vec3 emission;
};

#ifdef USE_INPUT_MATERIAL
struct textured_material_t
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

uniform textured_material_t input_material;

vec4 get_material_color()
{
#ifdef MATERIAL_COLOR_TEXTURE
    return texture(
        input_material.color,
#ifdef VERTEX_UV0
        f_in.uv
#else
        vec2(0.0f)
#endif
    ) * input_material.color_factor;
#else
    return input_material.color_factor;
#endif
}

vec2 get_material_metallic_roughness()
{
#ifdef MATERIAL_METALLIC_ROUGHNESS_TEXTURE
    return texture(
        input_material.metallic_roughness,
#ifdef VERTEX_UV0
        f_in.uv
#else
        vec2(0.0f)
#endif
    ).bg * vec2(
        input_material.metallic_factor,
        input_material.roughness_factor
    );
#else
    return vec2(
        input_material.metallic_factor,
        input_material.roughness_factor
    );
#endif
}

#ifdef MATERIAL_NORMAL_TEXTURE
vec3 get_material_normal()
{
    return normalize(texture(
        input_material.normal,
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
        input_material.emission,
#ifdef VERTEX_UV0
        f_in.uv
#else
        vec2(0.0f)
#endif
    ).xyz * input_material.emission_factor;
#else
    return input_material.emission_factor;
#endif
}

float get_material_f0()
{
    return input_material.f0;
}

material_t get_input_material()
{
    material_t m;
    m.color = get_material_color();
    vec2 mr = get_material_metallic_roughness();
    m.metallic = mr.x;
    m.roughness = mr.y;
#ifdef MATERIAL_NORMAL_TEXTURE
    m.normal = get_material_normal();
#else
    m.normal = vec3(0,0,1);
#endif
    m.f0 = get_material_f0();
    m.emission = get_material_emission();
    return m;
}
#endif
