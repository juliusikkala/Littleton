#ifndef CAMERA_HH
#define CAMERA_HH
#include "transformable.hh"
#include "math.hh"

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

    float get_near() const;
    float get_far() const;
    float get_aspect() const;

    // pos must be in view space
    bool sphere_is_visible(glm::vec3 pos, float r) const;

    glm::vec2 pixels_per_unit(glm::uvec2 target_size) const;

private:
    glm::mat4 projection;
    glm::vec3 clip_info;
    glm::vec2 projection_info;

    float near, far, aspect;
    float tan_fov;
    glm::vec2 inverse_cos_fov;
};

#endif
