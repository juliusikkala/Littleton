#include "transformable.hh"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

transformable::transformable()
: position(0), scaling(1)
{
}

transformable::transformable(const transformable& other)
: orientation(other.orientation), position(other.position),
  scaling(other.scaling)
{}

void transformable::rotate(float angle, glm::vec3 axis, glm::vec3 local_origin)
{
    glm::quat rotation = glm::angleAxis(angle, axis);
    orientation = rotation * orientation;
    position += local_origin + rotation * -local_origin;
}

void transformable::rotate(glm::quat rotation)
{
    orientation = rotation * orientation;
}

void transformable::set_orientation(float angle, glm::vec3 axis)
{
    orientation = glm::angleAxis(angle, axis);
}

void transformable::set_orientation(glm::quat orientation)
{
    this->orientation = orientation;
}

glm::quat transformable::get_orientation() const { return orientation; }

void transformable::translate(glm::vec3 offset)
{
    this->position += offset;
}

void transformable::set_position(glm::vec3 position)
{
    this->position = position;
}

glm::vec3 transformable::get_position() const { return position; }

void transformable::scale(glm::vec3 scale)
{
    this->scaling *= scale;
}

void transformable::set_scaling(glm::vec3 scaling)
{
    this->scaling = scaling;
}
glm::vec3 transformable::get_scaling() const { return scaling; }

void transformable::set_transform(const glm::mat4& transform)
{
    position = transform[3];
    scaling = glm::vec3(
        glm::length(transform[0]),
        glm::length(transform[1]),
        glm::length(transform[2])
    );
    orientation = glm::quat(glm::mat4(
        transform[0]/scaling.x,
        transform[1]/scaling.y,
        transform[2]/scaling.z,
        glm::vec4(0,0,0,1)
    ));
}

glm::mat4 transformable::get_transform() const
{
    return glm::translate(position) * glm::toMat4(orientation)
        * glm::scale(scaling);
}

transformable_node::transformable_node(transformable_node* parent)
: parent(parent) {}

glm::mat4 transformable_node::get_global_transform() const 
{
    return parent ?
        get_transform() * parent->get_global_transform() :
        get_transform();
}

void transformable_node::set_parent(transformable_node* parent)
{
    this->parent = parent;
}

transformable_node* transformable_node::get_parent() const
{
    return parent;
}
