/*
    Copyright 2018 Julius Ikkala

    This file is part of Littleton.

    Littleton is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Littleton is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Littleton.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef LT_SCENE_HH
#define LT_SCENE_HH
#include "api.hh"
#include <vector>
#include <map>
#include "math.hh"

namespace lt
{

class object;
class camera;
class light;
class directional_light;
class point_light;
class spotlight;
class directional_shadow_map;
class omni_shadow_map;
class perspective_shadow_map;
class environment_map;

namespace method { class shadow_method; }

class LT_API camera_scene
{
public:
    camera_scene(camera* cam = nullptr);
    ~camera_scene();

    void set_camera(camera* cam);
    camera* get_camera() const;

private:
    camera* cam;
};

class LT_API object_scene
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

class LT_API light_scene
{
public:
    light_scene(
        std::vector<point_light*>&& point_lights = {},
        std::vector<spotlight*>&& spotlights = {},
        std::vector<directional_light*>&& directional_lights = {}
    );
    ~light_scene();

    void set_ambient(glm::vec3 ambient);
    glm::vec3 get_ambient() const;

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
    glm::vec3 ambient;
    std::vector<point_light*> point_lights;
    std::vector<spotlight*> spotlights;
    std::vector<directional_light*> directional_lights;
};

class LT_API shadow_scene
{
public:
    shadow_scene();
    ~shadow_scene();

    using directional_map = std::map<
        method::shadow_method*,
        std::vector<directional_shadow_map*>
    >;

    using omni_map = std::map<
        method::shadow_method*,
        std::vector<omni_shadow_map*>
    >;

    using perspective_map = std::map<
        method::shadow_method*,
        std::vector<perspective_shadow_map*>
    >;

    void add_shadow(directional_shadow_map* shadow);
    void remove_shadow(directional_shadow_map* shadow);

    void add_shadow(omni_shadow_map* shadow);
    void remove_shadow(omni_shadow_map* shadow);

    void add_shadow(perspective_shadow_map* shadow);
    void remove_shadow(perspective_shadow_map* shadow);

    void clear_directional_shadows();
    void clear_omni_shadows();
    void clear_perspective_shadows();
    void clear_shadows();

    const directional_map& get_directional_shadows() const;
    const omni_map& get_omni_shadows() const;
    const perspective_map& get_perspective_shadows() const;

private:
    directional_map directional_shadows;
    omni_map omni_shadows;
    perspective_map perspective_shadows;
};

class LT_API environment_scene
{
public:
    environment_scene();

    void set_skybox(environment_map* skybox);
    environment_map* get_skybox() const;

private:
    environment_map* skybox;
};

class LT_API render_scene
: public camera_scene, public object_scene, public light_scene,
  public shadow_scene, public environment_scene
{
public:
    render_scene();
};

} // namespace lt

#endif
