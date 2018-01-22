#ifndef OBJECT_HH
#define OBJECT_HH
#include "model.hh"
#include "transformable.hh"
#include <glm/glm.hpp>

class object: public transformable_node
{
public:
    object(model* mod = nullptr, transformable_node* parent = nullptr);
    ~object();

    /* Be extra careful when using this function. Make sure that 'model'
     * outlives this object or is unset before its destruction.
     */
    void set_model(model* mod = nullptr);
    model* get_model() const;

private:
    model* mod;
};

#endif

