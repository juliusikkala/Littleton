#include "object.hh"

object::object()
: transform(1) {}

object::~object() {}

void object::set_transform(const glm::mat4& transform)
{
    this->transform = transform;
}

glm::mat4 object::get_transform() { return transform; }

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
