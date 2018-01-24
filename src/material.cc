#include "material.hh"
#include "texture.hh"

material::material(): ior(1.0) {}

shader::definition_map material::get_definitions() const
{
    shader::definition_map definitions;

    if(metallic.index() == 0)
    {
        if(std::get<0>(metallic)) definitions["MATERIAL_METALLIC_TEXTURE"];
    } else definitions["MATERIAL_METALLIC_CONSTANT"];

    if(color.index() == 0)
    {
        if(std::get<0>(color)) definitions["MATERIAL_COLOR_TEXTURE"];
    } else definitions["MATERIAL_COLOR_CONSTANT"];

    if(roughness.index() == 0)
    {
        if(std::get<0>(roughness)) definitions["MATERIAL_ROUGHNESS_TEXTURE"];
    } else definitions["MATERIAL_ROUGHNESS_CONSTANT"];

    if(normal) definitions["MATERIAL_NORMAL_TEXTURE"];

    if(emission.index() == 0)
    {
        if(std::get<0>(emission)) definitions["MATERIAL_EMISSION_TEXTURE"];
    } else definitions["MATERIAL_EMISSION_CONSTANT"];

    if(subsurface_scattering.index() == 0)
    {
        if(std::get<0>(subsurface_scattering))
            definitions["MATERIAL_SUBSURFACE_SCATTERING_TEXTURE"];
    } else definitions["MATERIAL_SUBSURFACE_SCATTERING_CONSTANT"];

    if(roughness.index() == 0)
    {
        if(std::get<0>(subsurface_depth))
            definitions["MATERIAL_SUBSURFACE_DEPTH_TEXTURE"];
    } else definitions["MATERIAL_SUBSURFACE_DEPTH_CONSTANT"];

    return definitions;
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

    s->set("material.ior", ior);

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
