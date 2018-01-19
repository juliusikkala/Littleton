#ifndef CAMERA_HH
#define CAMERA_HH
#include "transformable.hh"
#include <glm/glm.hpp>

class camera: public transformable
{
public:
    camera();
    ~camera();

    glm::mat4 get_perspective() const;

private:
    glm::mat4 perspective;
};

#endif
