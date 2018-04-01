#include "scene.hh"
#include "helpers.hh"
#include "light.hh"
#include "shadow_map.hh"

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

object_scene::object_scene(std::vector<object*>&& objects)
: objects(std::move(objects)) {}

object_scene::~object_scene() {}

void object_scene::add_object(object* obj)
{
    sorted_insert(objects, obj);
}

void object_scene::remove_object(object* obj)
{
    sorted_erase(objects, obj);
}

void object_scene::clear_objects()
{
    objects.clear();
}

size_t object_scene::object_count() const
{
    return objects.size();
}

void object_scene::set_objects(const std::vector<object*>& objects)
{
    this->objects = objects;
}

const std::vector<object*>& object_scene::get_objects() const
{
    return objects;
}

light_scene::light_scene(
    std::vector<point_light*>&& point_lights,
    std::vector<spotlight*>&& spotlights,
    std::vector<directional_light*>&& directional_lights
):  ambient(0), point_lights(std::move(point_lights)),
    spotlights(std::move(spotlights)),
    directional_lights(std::move(directional_lights)) {}

light_scene::~light_scene() {}

void light_scene::set_ambient(glm::vec3 ambient)
{
    this->ambient = ambient;
}

glm::vec3 light_scene::get_ambient() const
{
    return ambient;
}

void light_scene::add_light(point_light* pl)
{
    sorted_insert(point_lights, pl);
}

void light_scene::add_light(spotlight* sp)
{
    sorted_insert(spotlights, sp);
}

void light_scene::add_light(directional_light* dl)
{
    sorted_insert(directional_lights, dl);
}

void light_scene::remove_light(point_light* pl)
{
    sorted_erase(point_lights, pl);
}

void light_scene::remove_light(spotlight* sp)
{
    sorted_erase(spotlights, sp);
}

void light_scene::remove_light(directional_light* dl)
{
    sorted_erase(directional_lights, dl);
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

void light_scene::set_point_lights(const std::vector<point_light*>& point_lights)
{
    this->point_lights = point_lights;
}

void light_scene::set_spotlights(const std::vector<spotlight*>& spotlights)
{
    this->spotlights = spotlights;
}

void light_scene::set_directional_lights(
    const std::vector<directional_light*>& directional_lights
){
    this->directional_lights = directional_lights;
}

std::vector<light*> light_scene::get_lights() const
{
    std::vector<light*> lights;
    lights.insert(lights.end(), point_lights.begin(), point_lights.end());
    lights.insert(lights.end(), spotlights.begin(), spotlights.end());
    lights.insert(
        lights.end(),
        directional_lights.begin(),
        directional_lights.end()
    );
    return lights;
}

const std::vector<point_light*>& light_scene::get_point_lights() const
{
    return point_lights;
}

const std::vector<spotlight*>& light_scene::get_spotlights() const
{
    return spotlights;
}

const std::vector<directional_light*>& light_scene::get_directional_lights() const
{
    return directional_lights;
}

shadow_scene::shadow_scene() {}
shadow_scene::~shadow_scene() {}

void shadow_scene::add_shadow(directional_shadow_map* shadow)
{
    method::shadow_method* method = shadow->get_method();
    sorted_insert(directional_shadows[method], shadow);
}

void shadow_scene::remove_shadow(directional_shadow_map* shadow)
{
    method::shadow_method* method = shadow->get_method();
    auto it = directional_shadows.find(method);
    if(it == directional_shadows.end()) return;

    sorted_erase(it->second, shadow);
    if(it->second.size() == 0) directional_shadows.erase(it);
}

void shadow_scene::add_shadow(omni_shadow_map* shadow)
{
    method::shadow_method* method = shadow->get_method();
    sorted_insert(omni_shadows[method], shadow);
}

void shadow_scene::remove_shadow(omni_shadow_map* shadow)
{
    method::shadow_method* method = shadow->get_method();
    auto it = omni_shadows.find(method);
    if(it == omni_shadows.end()) return;

    sorted_erase(it->second, shadow);
    if(it->second.size() == 0) omni_shadows.erase(it);
}

void shadow_scene::add_shadow(perspective_shadow_map* shadow)
{
    method::shadow_method* method = shadow->get_method();
    sorted_insert(perspective_shadows[method], shadow);
}

void shadow_scene::remove_shadow(perspective_shadow_map* shadow)
{
    method::shadow_method* method = shadow->get_method();
    auto it = perspective_shadows.find(method);
    if(it == perspective_shadows.end()) return;

    sorted_erase(it->second, shadow);
    if(it->second.size() == 0) perspective_shadows.erase(it);
}

void shadow_scene::clear_directional_shadows()
{
    directional_shadows.clear();
}

void shadow_scene::clear_omni_shadows()
{
    omni_shadows.clear();
}

void shadow_scene::clear_perspective_shadows()
{
    perspective_shadows.clear();
}

void shadow_scene::clear_shadows()
{
    directional_shadows.clear();
    omni_shadows.clear();
    perspective_shadows.clear();
}

const shadow_scene::directional_map& shadow_scene::get_directional_shadows() const
{
    return directional_shadows;
}

const shadow_scene::omni_map& shadow_scene::get_omni_shadows() const
{
    return omni_shadows;
}

const shadow_scene::perspective_map&
shadow_scene::get_perspective_shadows() const
{
    return perspective_shadows;
}

environment_scene::environment_scene()
: skybox(nullptr) {}

void environment_scene::set_skybox(environment_map* skybox)
{
    this->skybox = skybox;
}

environment_map* environment_scene::get_skybox() const
{
    return skybox;
}

render_scene::render_scene() {}
