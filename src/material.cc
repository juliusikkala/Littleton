#include "material.hh"
#include "texture.hh"

material::material(): ior(1.0) {}

template<typename T>
static void update_texture_variant_def(
    shader::definition_map& def,
    const T& v,
    const char* constant_key,
    const char* texture_key
){
    if(v.index() == 0)
    {
        def.erase(constant_key);
        if(std::get<0>(v)) def[texture_key];
        else def.erase(texture_key);
    }
    else
    {
        def.erase(texture_key);
        def[constant_key];
    }
}

void material::update_definitions(shader::definition_map& def) const
{
    update_texture_variant_def(
        def,
        metallic,
        "MATERIAL_METALLIC_CONSTANT",
        "MATERIAL_METALLIC_TEXTURE"
    );

    update_texture_variant_def(
        def,
        color,
        "MATERIAL_COLOR_CONSTANT",
        "MATERIAL_COLOR_TEXTURE"
    );

    update_texture_variant_def(
        def,
        roughness,
        "MATERIAL_ROUGHNESS_CONSTANT",
        "MATERIAL_ROUGHNESS_TEXTURE"
    );

    if(normal) def["MATERIAL_NORMAL_TEXTURE"];
    else def.erase("MATERIAL_NORMAL_TEXTURE");

    update_texture_variant_def(
        def,
        emission,
        "MATERIAL_EMISSION_CONSTANT",
        "MATERIAL_EMISSION_TEXTURE"
    );

    update_texture_variant_def(
        def,
        subsurface_scattering,
        "MATERIAL_SUBSURFACE_SCATTERING_CONSTANT",
        "MATERIAL_SUBSURFACE_SCATTERING_TEXTURE"
    );

    update_texture_variant_def(
        def,
        subsurface_depth,
        "MATERIAL_SUBSURFACE_DEPTH_CONSTANT",
        "MATERIAL_SUBSURFACE_DEPTH_TEXTURE"
    );
}

void material::apply(shader* s)
{
    if(metallic.index() == 0)
    {
        texture* tex = std::get<0>(metallic);
        if(tex)
        {
            tex->bind(0);
            s->set("material.metallic", 0);
        }
    } else s->set("material.metallic", std::get<1>(metallic));

    if(color.index() == 0)
    {
        texture* tex = std::get<0>(color);
        if(tex)
        {
            tex->bind(1);
            s->set("material.color", 1);
        }
    } else s->set("material.color", std::get<1>(color));

    if(roughness.index() == 0)
    {
        texture* tex = std::get<0>(roughness);
        if(tex)
        {
            tex->bind(2);
            s->set("material.roughness", 2);
        }
    } else s->set("material.roughness", std::get<1>(roughness));

    if(normal)
    {
        normal->bind(3);
        s->set("material.normal", 3);
    }

    s->set<float>("material.f0", 2 * pow((ior-1)/(ior+1), 2));

    if(emission.index() == 0)
    {
        texture* tex = std::get<0>(emission);
        if(tex)
        {
            tex->bind(4);
            s->set("material.emission", 4);
        }
    } else s->set("material.emission", std::get<1>(emission));

    if(subsurface_scattering.index() == 0)
    {
        texture* tex = std::get<0>(subsurface_scattering);
        if(tex)
        {
            tex->bind(5);
            s->set("material.subsurface_scattering", 5);
        }
    } else s->set(
        "material.subsurface_scattering",
        std::get<1>(subsurface_scattering)
    );

    if(subsurface_depth.index() == 0)
    {
        texture* tex = std::get<0>(subsurface_depth);
        if(tex)
        {
            tex->bind(6);
            s->set("material.subsurface_depth", 6);
        }
    } else s->set(
        "material.subsurface_depth",
        std::get<1>(subsurface_depth)
    );
}
