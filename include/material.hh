#ifndef LT_MATERIAL_HH
#define LT_MATERIAL_HH
#include "shader.hh"
#include "sampler.hh"
#include "math.hh"
#include <variant>

namespace lt
{

class texture;

class material
{
public:
    material();

    void update_definitions(shader::definition_map& def) const;
    void apply(shader* s, unsigned& texture_index) const;

    bool potentially_transparent() const;

    using sampler_tex = std::pair<const sampler*, const texture*>;

    glm::vec4 color_factor;
    sampler_tex color_texture;

    float metallic_factor;
    float roughness_factor;
    sampler_tex metallic_roughness_texture;

    float normal_factor;
    sampler_tex normal_texture;

    float ior;

    glm::vec3 emission_factor;
    sampler_tex emission_texture;
};

} // namespace lt

#endif

