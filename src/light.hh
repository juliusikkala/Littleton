#ifndef POINT_LIGHT_HH
#define POINT_LIGHT_HH
#include "transformable.hh"

class light
{
public:
    light(glm::vec3 color = glm::vec3(1.0));

    void set_color(glm::vec3 color);
    glm::vec3 get_color() const;

private:
    glm::vec3 color;
};

class directional_light: public light
{
public:
    directional_light(
        glm::vec3 color = glm::vec3(1.0),
        glm::vec3 direction = glm::vec3(0,-1,0)
    );

    void set_direction(glm::vec3 color);
    glm::vec3 get_direction() const;

private:
    glm::vec3 direction;
};

class point_light: public light, public transformable_node
{
public:
    point_light(glm::vec3 color = glm::vec3(1.0));
};

class spotlight: public point_light
{
public:
    spotlight(
        glm::vec3 color = glm::vec3(1.0),
        float cutoff_angle = 30,
        float falloff_exponent = 1
    );

    void set_cutoff_angle(float cutoff_angle);
    float get_cutoff_angle() const;

    void set_falloff_exponent(float falloff_exponent);
    float get_falloff_exponent() const;

    glm::vec3 get_global_direction() const;

private:
    float cutoff_angle;
    float falloff_exponent;
};

#endif
