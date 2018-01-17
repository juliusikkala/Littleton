#ifndef SCENE_HH
#define SCENE_HH
#include "resources.hh"
#include "object.hh"
#include "camera.hh"
#include <set>

class scene
{
public:
    scene();
    ~scene();

    void set_camera(const camera& cam);
    camera& get_camera();
    const camera& get_camera() const;

    void add(const object_ptr& obj);

    void remove(const object_ptr& obj);

    void clear();

    size_t object_count() const;

    using iterator = std::set<object_ptr>::iterator;
    using const_iterator = std::set<object_ptr>::const_iterator;

    iterator begin();
    const_iterator begin() const;

    iterator end();
    const_iterator end() const;

private:
    camera cam;
    std::set<object_ptr> objects;
};

#endif
