#ifndef OBJECT_HH
#define OBJECT_HH
#include "resources.hh"
#include "model.hh"
#include <glm/glm.hpp>

class object
{
public:
    object();
    ~object();

    void set_transform(const glm::mat4& transform);
    glm::mat4 get_transform() const;
    glm::mat4 get_global_transform() const;

    void set_parent(const resource_ptr<object>& parent);

    /* Be extra careful when using this function. Make sure that 'parent'
     * outlives this object or is unset before its destruction.
     */
    void set_parent(object& parent);

    /* Makes this a root object. */
    void set_parent();

    const resource_ptr<object>& get_parent() const;

    void set_model(const model_ptr& mod);

    /* Be extra careful when using this function. Make sure that 'model'
     * outlives this object or is unset before its destruction.
     */
    void set_model(model& mod);
    /* Removes the model. */
    void set_model();
    const model_ptr& get_model() const;

private:
    glm::mat4 transform;
    resource_ptr<object> parent;
    model_ptr mod;
};

using object_ptr = resource_ptr<object>;

#endif

