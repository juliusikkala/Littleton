#ifndef OBJECT_HH
#define OBJECT_HH
#include "model.hh"
#include "transformable.hh"
#include <glm/glm.hpp>

class object: public transformable
{
public:
    object(model* mod = nullptr, object* parent = nullptr);
    ~object();

    glm::mat4 get_global_transform() const;

    void set_parent(object* parent = nullptr);
    object* get_parent() const;

    /* Be extra careful when using this function. Make sure that 'model'
     * outlives this object or is unset before its destruction.
     */
    void set_model(model* mod = nullptr);
    model* get_model() const;

private:
    model* mod;
    object* parent;
};

#endif

