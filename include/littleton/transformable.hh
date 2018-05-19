/*
    Copyright 2018 Julius Ikkala

    This file is part of Littleton.

    Littleton is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Littleton is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Littleton.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef LT_TRANSFORMABLE_HH
#define LT_TRANSFORMABLE_HH
#include "api.hh"
#include "math.hh"

namespace lt
{

class LT_API transformable
{
public:
    transformable();
    transformable(const transformable& other);

    void rotate(
        float angle,
        glm::vec3 axis,
        glm::vec3 local_origin = glm::vec3(0)
    );

    void rotate_local(
        float angle,
        glm::vec3 axis,
        glm::vec3 local_origin = glm::vec3(0)
    );
    void rotate(glm::quat rotation);
    void set_orientation(float angle, glm::vec3 axis);
    void set_orientation(float pitch, float yaw, float roll = 0);
    void set_orientation(glm::quat orientation = glm::quat());
    glm::quat get_orientation() const;

    void translate(glm::vec3 offset); 
    void translate_local(glm::vec3 offset);
    void set_position(glm::vec3 position = glm::vec3(0));
    glm::vec3 get_position() const;

    void scale(float scale);
    void scale(glm::vec3 scale);
    void set_scaling(glm::vec3 scaling = glm::vec3(1));
    glm::vec3 get_scaling() const;

    void set_transform(const glm::mat4& transform);
    glm::mat4 get_transform() const;

    void lookat(
        glm::vec3 pos,
        glm::vec3 up = glm::vec3(0,1,0),
        glm::vec3 forward = glm::vec3(0,0,-1),
        float angle_limit = -1
    );
    void lookat(
        const transformable* other,
        glm::vec3 up = glm::vec3(0,1,0),
        glm::vec3 forward = glm::vec3(0,0,-1),
        float angle_limit = -1
    );

protected:
    glm::quat orientation;
    glm::vec3 position, scaling;
};

class transformable_node: public transformable
{
public:
    transformable_node(transformable_node* parent = nullptr);

    glm::mat4 get_global_transform() const;

    glm::vec3 get_global_position() const;
    glm::quat get_global_orientation() const;
    glm::vec3 get_global_scaling() const;

    void set_parent(transformable_node* parent = nullptr);
    transformable_node* get_parent() const;

    void lookat(
        glm::vec3 pos,
        glm::vec3 up = glm::vec3(0,1,0),
        glm::vec3 forward = glm::vec3(0,0,-1),
        float angle_limit = -1
    );
    void lookat(
        const transformable* other,
        glm::vec3 up = glm::vec3(0,1,0),
        glm::vec3 forward = glm::vec3(0,0,-1),
        float angle_limit = -1
    );
    void lookat(
        const transformable_node* other,
        glm::vec3 up = glm::vec3(0,1,0),
        glm::vec3 forward = glm::vec3(0,0,-1),
        float angle_limit = -1
    );

protected:
    transformable_node* parent;
};

} // namespace lt

#endif
