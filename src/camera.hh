#ifndef CAMERA_HH
#define CAMERA_HH
#include <glm/glm.hpp>

class camera
{
public:
    camera();
    ~camera();

    glm::mat4 get_view() const;
    glm::mat4 get_perspective() const;

private:
    glm::mat4 view, perspective;
    float fov;
};

#endif
