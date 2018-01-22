#include "object.hh"

object::object(model* mod, transformable_node* parent)
: transformable_node(parent), mod(mod) {}
object::~object() {}

void object::set_model(model* mod) { this->mod = mod; }
model* object::get_model() const { return mod; }
