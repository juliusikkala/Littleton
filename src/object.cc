#include "object.hh"

object::object(const model* mod, transformable_node* parent)
: transformable_node(parent), mod(mod) {}
object::~object() {}

void object::set_model(const model* mod) { this->mod = mod; }
const model* object::get_model() const { return mod; }
