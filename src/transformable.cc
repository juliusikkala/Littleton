#include "transformable.hh"
#include "helpers.hh"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

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
    glm::quat rotation = glm::angleAxis(glm::radians(angle), axis);
    orientation = glm::normalize(rotation * orientation);
    position += local_origin + rotation * -local_origin;
}

void transformable::rotate(glm::quat rotation)
{
    orientation = glm::normalize(rotation * orientation);
}

void transformable::set_orientation(float angle, glm::vec3 axis)
{
    orientation = glm::angleAxis(glm::radians(angle), axis);
}

void transformable::set_orientation(glm::quat orientation)
{
    this->orientation = orientation;
}

void transformable::set_orientation(float pitch, float yaw, float roll)
{
    this->orientation = glm::quat(
        glm::vec3(
            glm::radians(pitch),
            glm::radians(yaw),
            glm::radians(roll)
        )
    );
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

void transformable::scale(float scale)
{
    this->scaling *= scale;
}

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
    decompose_matrix(transform, position, scaling, orientation);
}

glm::mat4 transformable::get_transform() const
{
    return glm::translate(position) * glm::toMat4(orientation)
        * glm::scale(scaling);
}

void transformable::lookat(
    glm::vec3 pos,
    glm::vec3 up,
    glm::vec3 forward,
    float angle_limit
){
    glm::vec3 dir = pos - position;
    glm::quat target = quat_lookat(dir, up, forward);

    if(angle_limit < 0) orientation = target;
    else orientation = rotate_towards(orientation, target, angle_limit);
}

void transformable::lookat(
    const transformable* other,
    glm::vec3 up,
    glm::vec3 forward,
    float angle_limit
){
    lookat(other->position, up, forward, angle_limit);
}

transformable_node::transformable_node(transformable_node* parent)
: parent(parent) {}

glm::mat4 transformable_node::get_global_transform() const 
{
    return parent ?
        parent->get_global_transform() * get_transform():
        get_transform();
}

glm::vec3 transformable_node::get_global_position() const
{
    return get_matrix_translation(get_global_transform());
}

glm::quat transformable_node::get_global_orientation() const
{
    return get_matrix_orientation(get_global_transform());
}

glm::vec3 transformable_node::get_global_scaling() const
{
    return get_matrix_scaling(get_global_transform());
}

void transformable_node::set_parent(transformable_node* parent)
{
    this->parent = parent;
}

transformable_node* transformable_node::get_parent() const
{
    return parent;
}

void transformable_node::lookat(
    glm::vec3 pos,
    glm::vec3 up,
    glm::vec3 forward,
    float angle_limit
){
    glm::vec3 dir = pos - position;
    glm::quat global_orientation = quat_lookat(dir, up, forward);
    glm::quat target;

    if(parent)
    {
        target =
            glm::inverse(parent->get_global_orientation()) * global_orientation;
    }
    else
    {
        target = global_orientation;
    }

    if(angle_limit < 0) orientation = target;
    else orientation = rotate_towards(orientation, target, angle_limit);
}

void transformable_node::lookat(
    const transformable* other,
    glm::vec3 up,
    glm::vec3 forward,
    float angle_limit
){
    lookat(other->get_position(), up, forward, angle_limit);
}

void transformable_node::lookat(
    const transformable_node* other,
    glm::vec3 up,
    glm::vec3 forward,
    float angle_limit
){
    lookat(other->get_global_position(), up, forward, angle_limit);
}
