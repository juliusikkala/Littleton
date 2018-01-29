#ifndef SCENE_HH
#define SCENE_HH
#include "resources.hh"
#include "object.hh"
#include "light.hh"
#include "camera.hh"
#include <set>
#include <unordered_map>
#include <typeindex>

class camera_scene
{
public:
    camera_scene(camera* cam);
    ~camera_scene();

    void set_camera(camera* cam);
    camera* get_camera() const;

private:
    camera* cam;
};

class object_scene
{
public:
    object_scene(
        std::set<object*> objects = {}
    );
    ~object_scene();

    void add_object(object* obj);
    void remove_object(object* obj);
    void clear_objects();
    size_t object_count() const;

    void set_objects(const std::set<object*>& objects);
    const std::set<object*>& get_objects() const;

private:
    std::set<object*> objects;
};

class light_scene
{
public:
    light_scene(
        std::set<point_light*> point_lights = {},
        std::set<spotlight*> spotlights = {}
    );
    ~light_scene();

    void add_light(point_light* pl);
    void add_light(spotlight* sp);
    void remove_light(point_light* pl);
    void remove_light(spotlight* sp);

    void clear_lights();
    void clear_point_lights();
    void clear_spotlights();

    size_t light_count() const;
    size_t point_light_count() const;
    size_t spotlight_count() const;

    void set_point_lights(const std::set<point_light*>& point_lights);
    void set_spotlights(const std::set<spotlight*>& spotlights);
    const std::set<point_light*>& get_point_lights() const;
    const std::set<spotlight*>& get_spotlights() const;

private:
    std::set<point_light*> point_lights;
    std::set<spotlight*> spotlights;
};

class render_scene: public camera_scene, public object_scene, public light_scene
{
public:
    render_scene(camera* cam);
};

#endif
