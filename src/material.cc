#include "material.hh"
#include "texture.hh"
#include "shader.hh"

material::material()
:   color_factor(glm::vec4(1.0f)),
    color_texture(nullptr, nullptr),
    metallic_factor(0.0f),
    roughness_factor(1.0f),
    metallic_roughness_texture(nullptr, nullptr),
    normal_factor(1.0f),
    normal_texture(nullptr, nullptr),
    ior(1.45f),
    emission_factor(0.0f),
    emission_texture(nullptr, nullptr)
{}

static void update_def(
    shader::definition_map& def,
    const material::sampler_tex& v,
    const char* key
){
    if(v.first == nullptr) def.erase(key);
    else def[key];
}

void material::update_definitions(shader::definition_map& def) const
{
    update_def(def, color_texture, "MATERIAL_COLOR_TEXTURE");
    update_def(
        def,
        metallic_roughness_texture,
        "MATERIAL_METALLIC_ROUGHNESS_TEXTURE"
    );
    update_def(def, normal_texture, "MATERIAL_NORMAL_TEXTURE");
    update_def(def, emission_texture, "MATERIAL_EMISSION_TEXTURE");
}

void material::apply(shader* s) const
{
    s->set("material.color_factor", color_factor);
    if(color_texture.first) s->set(
        "material.color",
        color_texture.first->bind(*color_texture.second, 0)
    );

    s->set("material.metallic_factor", metallic_factor);
    s->set("material.roughness_factor", roughness_factor);
    if(metallic_roughness_texture.first) s->set(
        "material.metallic_roughness",
        metallic_roughness_texture.first->bind(
            *metallic_roughness_texture.second,
            1
        )
    );

    s->set("material.normal_factor", normal_factor);
    if(normal_texture.first) s->set(
        "material.normal",
        normal_texture.first->bind(*normal_texture.second, 2)
    );

    s->set<float>("material.f0", 2 * pow((ior-1)/(ior+1), 2));

    s->set("material.emission_factor", emission_factor);
    if(emission_texture.first) s->set(
        "material.emission",
        emission_texture.first->bind(*emission_texture.second, 3)
    );
}

bool material::potentially_transparent() const
{
    return color_factor.a < 1.0f || (color_texture.second && 
        color_texture.second->get_external_format() == GL_RGBA);
}
