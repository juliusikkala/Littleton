#include "material.hh"
#include "texture.hh"
#include "shader.hh"

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
        material::sampler_tex tex = std::get<0>(v);
        if(tex.first && tex.second) def[texture_key];
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

    if(normal.first && normal.second) def["MATERIAL_NORMAL_TEXTURE"];
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
        sampler_tex tex = std::get<0>(metallic);
        if(tex.first && tex.second)
        {
            s->set("material.metallic", tex.first->bind(*tex.second, 0));
        }
    } else s->set("material.metallic", std::get<1>(metallic));

    if(color.index() == 0)
    {
        sampler_tex tex = std::get<0>(color);
        if(tex.first && tex.second)
        {
            s->set("material.color", tex.first->bind(*tex.second, 1));
        }
    } else s->set("material.color", std::get<1>(color));

    if(roughness.index() == 0)
    {
        sampler_tex tex = std::get<0>(roughness);
        if(tex.first && tex.second)
        {
            s->set("material.roughness", tex.first->bind(*tex.second, 2));
        }
    } else s->set("material.roughness", std::get<1>(roughness));

    if(normal.first && normal.second)
    {
        s->set("material.normal", normal.first->bind(*normal.second, 3));
    }

    s->set<float>("material.f0", 2 * pow((ior-1)/(ior+1), 2));

    if(emission.index() == 0)
    {
        sampler_tex tex = std::get<0>(emission);
        if(tex.first && tex.second)
        {
            s->set("material.emission", tex.first->bind(*tex.second, 4));
        }
    } else s->set("material.emission", std::get<1>(emission));

    if(subsurface_scattering.index() == 0)
    {
        sampler_tex tex = std::get<0>(subsurface_scattering);
        if(tex.first && tex.second)
        {
            s->set(
                "material.subsurface_scattering",
                tex.first->bind(*tex.second, 5)
            );
        }
    } else s->set(
        "material.subsurface_scattering",
        std::get<1>(subsurface_scattering)
    );

    if(subsurface_depth.index() == 0)
    {
        sampler_tex tex = std::get<0>(subsurface_depth);
        if(tex.first && tex.second)
        {
            s->set(
                "material.subsurface_depth",
                tex.first->bind(*tex.second, 6)
            );
        }
    } else s->set(
        "material.subsurface_depth",
        std::get<1>(subsurface_depth)
    );
}

bool material::potentially_transparent() const
{
    if(color.index() == 0)
    {
        sampler_tex tex = std::get<0>(color);
        if(tex.second) return tex.second->get_external_format() == GL_RGBA;
        return false;
    } else return std::get<1>(color).a < 1.0f;
}
