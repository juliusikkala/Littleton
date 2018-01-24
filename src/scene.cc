#include "scene.hh"

scene::scene(
    camera* cam,
    std::set<object*> objects
): cam(cam), objects(objects) {}
scene::~scene() {}

void scene::set_camera(camera* cam) { this->cam = cam; }
camera* scene::get_camera() const { return cam; }

void scene::add(object* obj)
{
    objects.insert(obj);
}

void scene::remove(object* obj)
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

scene::iterator scene::begin() { return objects.begin(); }
scene::const_iterator scene::cbegin() const { return objects.cbegin(); }
scene::iterator scene::end() { return objects.end(); }
scene::const_iterator scene::cend() const { return objects.cend(); }
