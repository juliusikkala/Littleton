#ifndef MATERIAL_HH
#define MATERIAL_HH
#include <glm/glm.hpp>
#include <variant>
#include "shader.hh"
#include "sampler.hh"

class texture;

class material
{
public:
    material();

    void update_definitions(shader::definition_map& def) const;
    void apply(shader* s);

    bool potentially_transparent() const;

    using sampler_tex = std::pair<sampler*, texture*>;

    std::variant<sampler_tex, float> metallic;
    std::variant<sampler_tex, glm::vec4> color;
    std::variant<sampler_tex, float> roughness;
    float ior;
    sampler_tex normal;
    std::variant<sampler_tex, float> emission;
    std::variant<sampler_tex, glm::vec4> subsurface_scattering;
    std::variant<sampler_tex, float> subsurface_depth;
};

#endif

