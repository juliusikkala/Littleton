#ifndef HELPERS_HH
#define HELPERS_HH
#include <string>
#include <map>
#include <vector>
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

#define sign(x) ((x > 0) - (x < 0))

bool solve_quadratic(float a, float b, float c, float& x0, float& x1);

bool intersect_sphere(
    glm::vec3 pos,
    glm::vec3 dir,
    glm::vec3 origin,
    float radius,
    float& t0,
    float& t1
);

unsigned next_power_of_two(unsigned n);

// Note that this function is very slow. Please save your generated samples.
// Circular version
std::vector<glm::vec2> mitchell_best_candidate(
    float r,
    unsigned candidate_count,
    unsigned count
);

// Rectangular version
std::vector<glm::vec2> mitchell_best_candidate(
    float w,
    float h,
    unsigned candidate_count,
    unsigned count
);

std::vector<glm::vec2> grid_samples(
    unsigned w,
    unsigned h,
    float step
);

std::vector<glm::vec2> poisson_samples(
    float w,
    float h,
    float mindist
);

#include "helpers.tcc"

#endif
