#ifndef SCENE_HH
#define SCENE_HH
#include <vector>

class object;
class camera;
class light;
class directional_light;
class point_light;
class spotlight;

class camera_scene
{
public:
    camera_scene(camera* cam = nullptr);
    ~camera_scene();

    void set_camera(camera* cam);
    camera* get_camera() const;

private:
    camera* cam;
};

class object_scene
{
public:
    object_scene(std::vector<object*>&& objects = {});
    ~object_scene();

    void add_object(object* obj);
    void remove_object(object* obj);
    void clear_objects();
    size_t object_count() const;

    void set_objects(const std::vector<object*>& objects);
    const std::vector<object*>& get_objects() const;

private:
    std::vector<object*> objects;
};

class light_scene
{
public:
    light_scene(
        std::vector<point_light*>&& point_lights = {},
        std::vector<spotlight*>&& spotlights = {},
        std::vector<directional_light*>&& directional_lights = {}
    );
    ~light_scene();

    void add_light(point_light* pl);
    void remove_light(point_light* pl);
    void clear_point_lights();
    size_t point_light_count() const;
    void set_point_lights(const std::vector<point_light*>& point_lights);
    const std::vector<point_light*>& get_point_lights() const;

    void add_light(spotlight* sp);
    void remove_light(spotlight* sp);
    void clear_spotlights();
    size_t spotlight_count() const;
    void set_spotlights(const std::vector<spotlight*>& spotlights);
    const std::vector<spotlight*>& get_spotlights() const;

    void add_light(directional_light* dl);
    void remove_light(directional_light* dl);
    void clear_directional_lights();
    size_t directional_light_count() const;
    void set_directional_lights(
        const std::vector<directional_light*>& directional_lights
    );
    const std::vector<directional_light*>& get_directional_lights() const;

    void clear_lights();
    size_t light_count() const;
    std::vector<light*> get_lights() const;

private:
    std::vector<point_light*> point_lights;
    std::vector<spotlight*> spotlights;
    std::vector<directional_light*> directional_lights;
};


class render_scene
: public camera_scene, public object_scene, public light_scene
{
public:
    render_scene();
};

#endif
