#ifndef MATERIAL_HH
#define MATERIAL_HH
#include "resources.hh"
#include "glheaders.hh"
#include "texture.hh"
#include <glm/glm.hpp>
#include <variant>

struct material
{
public:
    material();

    std::variant<texture_ptr, float> metallic;
    std::variant<texture_ptr, glm::vec4> color;
    std::variant<texture_ptr, float> roughness;
    float ior;
    texture_ptr normal;
    std::variant<texture_ptr, float> emission;
    std::variant<texture_ptr, glm::vec4> subsurface_scattering;
    std::variant<texture_ptr, float> subsurface_depth;
};

using material_ptr = resource_ptr<material>;

#endif

