#ifndef LT_MATH_HH
#define LT_MATH_HH
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

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
unsigned factorize(unsigned n);

// Computes a modelview matrix for a quad such that it completely covers the
// surface area of a sphere. use_near_radius determines whether the resulting
// depth value is picked from the near edge of the sphere or the furthest edge.
glm::mat4 sphere_projection_quad_matrix(
    glm::vec3 pos,
    float r,
    float near,
    float far,
    bool use_near_radius = false,
    float big = 1e3
);

// Note that this function is very slow. Please save your generated samples.
// If samples already contain some values, they're assumed to be generated by
// a previous call to this function with the same r.
// Circular version
void mitchell_best_candidate(
    std::vector<glm::vec2>& samples,
    float r,
    unsigned candidate_count,
    unsigned count
);

// Rectangular version
void mitchell_best_candidate(
    std::vector<glm::vec2>& samples,
    float w,
    float h,
    unsigned candidate_count,
    unsigned count
);

// Spherical version
void mitchell_best_candidate(
    std::vector<glm::vec3>& samples,
    float r,
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

std::vector<float> generate_gaussian_kernel(
    int radius,
    float sigma
);


#endif
