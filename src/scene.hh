#ifndef SCENE_HH
#define SCENE_HH
#include "resources.hh"
#include "object.hh"
#include "camera.hh"
#include <set>

class scene
{
public:
    scene(
        camera* cam,
        std::set<object*> objects = {}
    );
    ~scene();

    void set_camera(camera* cam);
    camera* get_camera() const;

    void add(object* obj);
    void remove(object* obj);

    void clear();

    size_t object_count() const;

    using iterator = std::set<object*>::iterator;
    using const_iterator = std::set<object*>::const_iterator;

    iterator begin();
    const_iterator cbegin() const;

    iterator end();
    const_iterator cend() const;

private:
    camera* cam;
    std::set<object*> objects;
};

#endif
