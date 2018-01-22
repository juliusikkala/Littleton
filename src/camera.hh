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
    void orthographic(float w, float h);
    void orthographic(float w, float h, float near, float far);
    glm::mat4 get_projection() const;

private:
    glm::mat4 projection;
};

#endif
