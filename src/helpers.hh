#ifndef HELPERS_HH
#define HELPERS_HH
#include <string>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

std::string read_text_file(const std::string& path);

template<typename T>
void hash_combine(std::size_t& seed, const T& v);

template<
    class K,
    class T,
    class C = std::less<K>,
    class A = std::allocator<std::pair<const K, T>>
>
struct map_hasher
{
    size_t operator()(const std::map<K, T, C, A>& arg) const noexcept
    {
        size_t hash = arg.size();
        for(auto& pair: arg)
        {
            hash_combine(hash, pair.first);
            hash_combine(hash, pair.second);
        }
        return hash;
    }
};

void decompose_matrix(
    const glm::mat4& transform,
    glm::vec3& translation,
    glm::vec3& scaling,
    glm::quat& orientation
);

glm::vec3 get_matrix_translation(const glm::mat4& transform);
glm::vec3 get_matrix_scaling(const glm::mat4& transform);
glm::quat get_matrix_orientation(const glm::mat4& transform);

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
