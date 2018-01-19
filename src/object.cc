#include "object.hh"

object::object() {}

object::~object() {}

glm::mat4 object::get_global_transform() const
{
    return parent ?
        get_transform() * parent->get_global_transform() :
        get_transform();
}

void object::set_parent(const resource_ptr<object>& parent)
{
    this->parent = parent;
}

void object::set_parent(object& parent)
{
    this->parent = parent;
}

void object::set_parent() { this->parent = nullptr; }

const resource_ptr<object>& object::get_parent() const { return parent; }

void object::set_model(const model_ptr& mod) { this->mod = mod; }

void object::set_model(model& mod) { this->mod = mod; }

void object::set_model() { mod = nullptr; }

const model_ptr& object::get_model() const { return mod; }
