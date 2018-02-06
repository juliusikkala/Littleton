#ifndef HELPERS_HH
#define HELPERS_HH
#include <string>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

std::string read_text_file(const std::string& path);

void decompose_matrix(
    const glm::mat4& transform,
    glm::vec3& translation,
    glm::vec3& scaling,
    glm::quat& orientation
);

glm::vec3 get_matrix_translation(const glm::mat4& transform);
glm::vec3 get_matrix_scaling(const glm::mat4& transform);
glm::quat get_matrix_orientation(const glm::mat4& transform);

void decompose_perspective(
    const glm::mat4& perspective,
    float& near,
    float& far,
    float& fov,
    float& aspect
);

glm::quat rotate_towards(
    glm::quat orig,
    glm::quat dest,
    float angle_limit
);

glm::quat quat_lookat(
    glm::vec3 dir,
    glm::vec3 up,
    glm::vec3 forward = glm::vec3(0,0,-1)
);

#include "helpers.tcc"

#endif
