#include "object.hh"

object::object(model* mod, object* parent): mod(mod), parent(parent) {}
object::~object() {}

glm::mat4 object::get_global_transform() const
{
    return parent ?
        get_transform() * parent->get_global_transform() :
        get_transform();
}
void object::set_parent(object* parent) { this->parent = parent; }
object* object::get_parent() const { return parent; }
void object::set_model(model* mod) { this->mod = mod; }
model* object::get_model() const { return mod; }
