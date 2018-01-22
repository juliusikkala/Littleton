#include "camera.hh"
#include <glm/gtc/matrix_transform.hpp>

camera::camera(transformable_node* parent): transformable_node(parent) {}
camera::~camera() {}

void camera::perspective(float fov, float aspect, float near)
{
    projection = glm::infinitePerspective(fov, aspect, near);
}

void camera::perspective(float fov, float aspect, float near, float far)
{
    projection = glm::perspective(fov, aspect, near, far);
}

void camera::orthographic(float w, float h)
{
    projection = glm::ortho(-w/2, w/2, -h/2, h/2);
}

void camera::orthographic(float w, float h, float near, float far)
{
    projection = glm::ortho(-w/2, w/2, -h/2, h/2, near, far);
}

glm::mat4 camera::get_projection() const
{
    return projection;
}
