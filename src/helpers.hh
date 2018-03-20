#ifndef HELPERS_HH
#define HELPERS_HH
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <boost/functional/hash.hpp>
#include "glheaders.hh"

std::string read_text_file(const std::string& path);
bool read_binary_file(const std::string& path, uint8_t*& data, size_t& bytes);
bool write_binary_file(const std::string& path, const uint8_t* data, size_t bytes);

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

template<typename T, typename Hash = boost::hash<T>>
std::string append_hash_to_path(
    const std::string& prefix,
    const T& hashable,
    const std::string& suffix = ""
);

size_t count_lines(const std::string& str);

std::string add_line_numbers(const std::string& src);

GLint internal_format_to_external_format(GLint internal_format);
GLint internal_format_compatible_type(GLint internal_format);

template<typename Resource, typename Pool>
class loan_returner
{
public:
    loan_returner();
    loan_returner(Pool& return_target);

    void operator()(Resource* res);

private:
    Pool* return_target;
};


template<typename Resource, typename Pool>
using loaner = std::unique_ptr<Resource, loan_returner<Resource, Pool>>;

template<typename T>
void sorted_insert(
    std::vector<T>& vec,
    const T& value
);

template<typename T>
bool sorted_erase(
    std::vector<T>& vec,
    const T& value
);

#include "helpers.tcc"

#endif
