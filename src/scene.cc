#include "scene.hh"

scene::scene() {}
scene::~scene() {}

void scene::set_camera(const camera& cam) { this->cam = cam; }
camera& scene::get_camera() { return cam; }
const camera& scene::get_camera() const { return cam; }

void scene::add(const object_ptr& obj)
{
    objects.insert(obj);
}

void scene::remove(const object_ptr& obj)
{
    objects.erase(obj);
}

void scene::clear()
{
    objects.clear();
}

size_t scene::object_count() const
{
    return objects.size();
}

scene::iterator scene::begin() { return objects.end(); }
scene::const_iterator scene::begin() const { return objects.end(); }
scene::iterator scene::end() { return objects.end(); }
scene::const_iterator scene::end() const { return objects.end(); }
