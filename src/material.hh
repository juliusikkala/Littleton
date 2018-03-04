#ifndef MATERIAL_HH
#define MATERIAL_HH
#include <glm/glm.hpp>
#include <variant>
#include "shader.hh"

class texture;

class material
{
public:
    material();

    void update_definitions(shader::definition_map& def) const;
    void apply(shader* s);

    std::variant<texture*, float> metallic;
    std::variant<texture*, glm::vec4> color;
    std::variant<texture*, float> roughness;
    float ior;
    texture* normal;
    std::variant<texture*, float> emission;
    std::variant<texture*, glm::vec4> subsurface_scattering;
    std::variant<texture*, float> subsurface_depth;
};

#endif

