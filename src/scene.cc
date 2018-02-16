#include "scene.hh"

camera_scene::camera_scene(camera* cam)
: cam(cam) {}

camera_scene::~camera_scene() {}

void camera_scene::set_camera(camera* cam)
{
    this->cam = cam;
}

camera* camera_scene::get_camera() const
{
    return cam;
}

object_scene::object_scene(std::set<object*> objects)
: objects(objects) {}

object_scene::~object_scene() {}

void object_scene::add_object(object* obj)
{
    objects.insert(obj);
}

void object_scene::remove_object(object* obj)
{
    objects.erase(obj);
}

void object_scene::clear_objects()
{
    objects.clear();
}

size_t object_scene::object_count() const
{
    return objects.size();
}

void object_scene::set_objects(const std::set<object*>& objects)
{
    this->objects = objects;
}

const std::set<object*>& object_scene::get_objects() const
{
    return objects;
}

light_scene::light_scene(
    std::set<point_light*> point_lights,
    std::set<spotlight*> spotlights,
    std::set<directional_light*> directional_lights
): point_lights(point_lights), spotlights(spotlights),
   directional_lights(directional_lights) {}
light_scene::~light_scene() {}

void light_scene::add_light(point_light* pl)
{
    point_lights.insert(pl);
}

void light_scene::add_light(spotlight* sp)
{
    spotlights.insert(sp);
}

void light_scene::add_light(directional_light* dl)
{
    directional_lights.insert(dl);
}

void light_scene::remove_light(point_light* pl)
{
    point_lights.erase(pl);
}

void light_scene::remove_light(spotlight* sp)
{
    spotlights.erase(sp);
}

void light_scene::remove_light(directional_light* dl)
{
    directional_lights.erase(dl);
}

void light_scene::clear_lights()
{
    clear_point_lights();
    clear_spotlights();
    clear_directional_lights();
}

void light_scene::clear_point_lights()
{
    point_lights.clear();
}

void light_scene::clear_spotlights()
{
    spotlights.clear();
}

void light_scene::clear_directional_lights()
{
    directional_lights.clear();
}

size_t light_scene::light_count() const
{
    return point_light_count() + spotlight_count() + directional_light_count();
}

size_t light_scene::point_light_count() const
{
    return point_lights.size();
}

size_t light_scene::spotlight_count() const
{
    return spotlights.size();
}

size_t light_scene::directional_light_count() const
{
    return directional_lights.size();
}

void light_scene::set_point_lights(const std::set<point_light*>& point_lights)
{
    this->point_lights = point_lights;
}

void light_scene::set_spotlights(const std::set<spotlight*>& spotlights)
{
    this->spotlights = spotlights;
}

void light_scene::set_directional_lights(
    const std::set<directional_light*>& directional_lights
){
    this->directional_lights = directional_lights;
}

const std::set<point_light*>& light_scene::get_point_lights() const
{
    return point_lights;
}

const std::set<spotlight*>& light_scene::get_spotlights() const
{
    return spotlights;
}

const std::set<directional_light*>& light_scene::get_directional_lights() const
{
    return directional_lights;
}

shadow_scene::shadow_scene(
    std::set<directional_shadow_map*> directional_shadow_maps
): directional_shadow_maps(std::move(directional_shadow_maps))
{
}

shadow_scene::~shadow_scene() {}

void shadow_scene::add_shadow_map(directional_shadow_map* dsm)
{
    directional_shadow_maps.insert(dsm);
}

void shadow_scene::remove_shadow_map(directional_shadow_map* dsm)
{
    directional_shadow_maps.erase(dsm);
}

void shadow_scene::clear_shadow_maps()
{
    clear_directional_shadow_maps();
}

void shadow_scene::clear_directional_shadow_maps()
{
    directional_shadow_maps.clear();
}

size_t shadow_scene::shadow_map_count() const
{
    return directional_shadow_map_count();
}

size_t shadow_scene::directional_shadow_map_count() const
{
    return this->directional_shadow_maps.size();
}

void shadow_scene::set_directional_shadow_maps(
    const std::set<directional_shadow_map*>& directional_shadow_maps
){
    this->directional_shadow_maps = directional_shadow_maps;
}

const std::set<directional_shadow_map*>&
shadow_scene::get_directional_shadow_maps() const
{
    return directional_shadow_maps;
}

render_scene::render_scene(camera* cam)
: camera_scene(cam) {}
