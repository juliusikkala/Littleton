#ifndef TRANSFORMABLE_HH
#define TRANSFORMABLE_HH
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class transformable
{
public:
    transformable();
    transformable(const transformable& other);

    void rotate(
        float angle,
        glm::vec3 axis,
        glm::vec3 local_origin = glm::vec3(0)
    );
    void rotate(glm::quat rotation);
    void set_orientation(float angle, glm::vec3 axis);
    void set_orientation(glm::quat orientation = glm::quat());
    glm::quat get_orientation() const;

    void translate(glm::vec3 offset); 
    void set_position(glm::vec3 position = glm::vec3(0));
    glm::vec3 get_position() const;

    void scale(glm::vec3 scale);
    void set_scaling(glm::vec3 scaling = glm::vec3(1));
    glm::vec3 get_scaling() const;

    void set_transform(const glm::mat4& transform);
    glm::mat4 get_transform() const;
    glm::mat4 get_inverse_transform() const;

private:
    glm::quat orientation;
    glm::vec3 position, scaling;
};

class transformable_node: public transformable
{
public:
    transformable_node(transformable_node* parent = nullptr);

    glm::mat4 get_global_transform() const;

    void set_parent(transformable_node* parent = nullptr);
    transformable_node* get_parent() const;

private:
    transformable_node* parent;
};

#endif
