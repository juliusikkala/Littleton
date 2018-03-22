#ifndef CAMERA_HH
#define CAMERA_HH
#include "transformable.hh"
#include <glm/glm.hpp>

class camera: public transformable_node
{
public:
    camera(transformable_node* parent = nullptr);
    ~camera();

    void perspective(float fov, float aspect, float near);
    void perspective(float fov, float aspect, float near, float far);
    glm::mat4 get_projection() const;
    glm::vec3 get_clip_info() const;
    glm::vec2 get_projection_info() const;

    glm::vec2 pixels_per_unit(glm::uvec2 target_size) const;

private:
    glm::mat4 projection;
    glm::vec3 clip_info;
    glm::vec2 projection_info;
};

#endif
