#ifndef MATERIAL_HH
#define MATERIAL_HH
#include "shader.hh"
#include <glm/glm.hpp>
#include <variant>

class texture;

struct material
{
public:
    material();

    shader::definition_map get_definitions() const;
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

