#ifndef SCENE_HH
#define SCENE_HH
#include "shadow/shadow_scene.hh"
#include <set>

class object;
class camera;
class directional_light;
class point_light;
class spotlight;

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
        std::set<spotlight*> spotlights = {},
        std::set<directional_light*> directional_lights = {}
    );
    ~light_scene();

    void add_light(point_light* pl);
    void add_light(spotlight* sp);
    void add_light(directional_light* dl);
    void remove_light(point_light* pl);
    void remove_light(spotlight* sp);
    void remove_light(directional_light* dl);

    void clear_lights();
    void clear_point_lights();
    void clear_spotlights();
    void clear_directional_lights();

    size_t light_count() const;
    size_t point_light_count() const;
    size_t spotlight_count() const;
    size_t directional_light_count() const;

    void set_point_lights(const std::set<point_light*>& point_lights);
    void set_spotlights(const std::set<spotlight*>& spotlights);
    void set_directional_lights(
        const std::set<directional_light*>& directional_lights
    );

    std::set<light*> get_lights() const;
    const std::set<point_light*>& get_point_lights() const;
    const std::set<spotlight*>& get_spotlights() const;
    const std::set<directional_light*>& get_directional_lights() const;

private:
    std::set<point_light*> point_lights;
    std::set<spotlight*> spotlights;
    std::set<directional_light*> directional_lights;
};

class render_scene
: public camera_scene, public object_scene, public light_scene,
  public shadow_scene
{
public:
    render_scene(camera* cam);
};

#endif
