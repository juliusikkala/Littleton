#include "camera.hh"
#include "helpers.hh"
#include <glm/gtc/matrix_transform.hpp>

camera::camera(transformable_node* parent): transformable_node(parent) {}
camera::~camera() {}

void camera::perspective(float fov, float aspect, float near)
{
    fov = glm::radians(fov);
    projection = glm::infinitePerspective(fov, aspect, near);
    clip_info = glm::vec3(near, -1, 1);
    projection_info = glm::vec2(2*tan(fov/2.0f)*aspect, 2*tan(fov/2.0f));
}

void camera::perspective(float fov, float aspect, float near, float far)
{
    fov = glm::radians(fov);
    projection = glm::perspective(fov, aspect, near, far);
    clip_info = glm::vec3(near * far, near - far, near + far);
    projection_info = glm::vec2(2*tan(fov/2.0f)*aspect, 2*tan(fov/2.0f));
}

glm::mat4 camera::get_projection() const
{
    return projection;
}

glm::vec3 camera::get_clip_info() const
{
    return clip_info;
}

glm::vec2 camera::get_projection_info() const
{
    return projection_info;
}

glm::vec2 camera::pixels_per_unit(glm::uvec2 target_size) const
{
    glm::vec3 top(1,1,-1);
    glm::vec3 bottom(-1,-1,-1);
    glm::vec4 top_projection = projection * glm::vec4(top, 1);
    top_projection /= top_projection.w;
    glm::vec4 bottom_projection = projection * glm::vec4(bottom, 1);
    bottom_projection /= bottom_projection.w;

    return 0.5f * glm::vec2(
        top_projection.x - bottom_projection.x,
        top_projection.y - bottom_projection.y
    ) * glm::vec2(target_size);
}
