/*
    Copyright 2018-2019 Julius Ikkala

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
#include "math.hh"
#include "timer.hh"
#include "spherical_gaussians.hh"
#include <vector>
#include <map>

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
class sg_group;
class sprite;

namespace method { class shadow_method; }

class LT_API camera_scene
{
public:
    explicit camera_scene(
        std::vector<camera*>&& cameras = {}
    );
    ~camera_scene();

    // These are for a single-camera setup ONLY, so set_camera clears all other
    // cameras and get_camera returns the first camera.
    void set_camera(camera* cam);
    camera* get_camera() const;

    // These are for multi-camera setups. Only useful for array render targets.
    void clear_cameras();
    size_t camera_count() const;
    void set_cameras(const std::vector<camera*>& cameras);
    const std::vector<camera*>& get_cameras() const;

    // Glue for composite_scene convenience functions, do not call directly.
    void clear_impl();

private:
    std::vector<camera*> cameras;
};

class LT_API object_scene
{
public:
    explicit object_scene(std::vector<object*>&& objects = {});
    ~object_scene();

    void add_object(object* obj);
    void remove_object(object* obj);
    void clear_objects();
    size_t object_count() const;

    void set_objects(const std::vector<object*>& objects);
    const std::vector<object*>& get_objects() const;

    // Glue for composite_scene convenience functions, do not call directly.
    void add_impl(object* obj);
    void remove_impl(object* obj);
    void clear_impl();

private:
    std::vector<object*> objects;
};

class LT_API sprite_scene
{
public:
    explicit sprite_scene(std::vector<sprite*>&& sprites = {});
    ~sprite_scene();

    void add_sprite(sprite* spr);
    void remove_sprite(sprite* spr);
    void clear_sprites();
    size_t sprite_count() const;

    void update_sprites(duration delta);

    void set_sprites(const std::vector<sprite*>& sprites);
    const std::vector<sprite*>& get_sprites() const;

    // Glue for composite_scene convenience functions, do not call directly.
    void add_impl(sprite* spr);
    void remove_impl(sprite* spr);
    void update_impl(duration delta);
    void clear_impl();

private:
    std::vector<sprite*> sprites;
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

    void set_ambient(vec3 ambient);
    vec3 get_ambient() const;

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

    // Glue for composite_scene convenience functions, do not call directly.
    void add_impl(point_light* pl);
    void remove_impl(point_light* pl);
    void add_impl(spotlight* sp);
    void remove_impl(spotlight* sp);
    void add_impl(directional_light* dl);
    void remove_impl(directional_light* dl);
    void clear_impl();

private:
    vec3 ambient;
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

    // Glue for composite_scene convenience functions, do not call directly.
    void add_impl(directional_shadow_map* shadow);
    void remove_impl(directional_shadow_map* shadow);
    void add_impl(omni_shadow_map* shadow);
    void remove_impl(omni_shadow_map* shadow);
    void add_impl(perspective_shadow_map* shadow);
    void remove_impl(perspective_shadow_map* shadow);
    void clear_impl();

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

    // Glue for composite_scene convenience functions, do not call directly.
    void clear_impl();

private:
    environment_map* skybox;
};

template<typename Scene>
class single_scene_holder
{
public:
    single_scene_holder(void* incorrect = nullptr): scene(nullptr) {}
    single_scene_holder(Scene* scene): scene(scene) {}

    void set(Scene* scene) { this->scene = scene; }
    Scene* get() const { return scene; }
private:
    Scene* scene;
};

// This class is designed to be used as follows:
// void myfunc(const scene_acceptor<a_scene, b_scene>& scene)
// {
//     a_scene* a = scene;
//     b_scene* b = scene;
// }
//
// ab_scene* ab;
// myfunc(ab);
//
// a_scene* a;
// b_scene* b;
// myfunc({ a, b });
//
// This allows the class to only depend on the needed scenes, but still take
// a single, composite scene as argument if available.
template<typename... Scenes>
struct scene_acceptor: public single_scene_holder<Scenes>...
{
    scene_acceptor() {}

    template<typename Scene>
    scene_acceptor(Scene* scene)
    : single_scene_holder<Scenes>(scene)... {}

    scene_acceptor(Scenes*... scenes)
    : single_scene_holder<Scenes>(scenes)... {}

    template<typename... OtherScenes>
    scene_acceptor(const scene_acceptor<OtherScenes...>& s)
    : single_scene_holder<Scenes>(s)... {}

    template<typename Scene>
    operator Scene*() const { return single_scene_holder<Scene>::get(); }
};

// If your method needs multiple scenes, use this to reduce boilerplate.
template<typename... Scenes>
struct scene_method: private single_scene_holder<Scenes>...
{
public:
    using SceneAcceptor = scene_acceptor<Scenes...>;
    using Scene = const scene_acceptor<Scenes...>&;

    template<typename S>
    S* get_scene() const { return single_scene_holder<S>::get(); }

    void set_scenes(Scene acceptor) { set_scenes<Scenes...>(acceptor); }

    template<typename S>
    void set_scene(S* scene) { single_scene_holder<S>::set(scene); }
    bool has_all_scenes() const { return has_scenes<Scenes...>(); }

    scene_acceptor<Scenes...> get_acceptor() const
    {
        scene_acceptor<Scenes...> acceptor;
        get_scenes<Scenes...>(acceptor);
        return acceptor;
    }

protected:
    scene_method(Scene acceptor)
    : single_scene_holder<Scenes>(acceptor)... {}

private:
    template<typename S, typename... Ss>
    void set_scenes(Scene acceptor)
    {
        set_scene<S>(acceptor);
        if constexpr(sizeof...(Ss) != 0) set_scenes<Ss...>(acceptor);
    }

    template<typename S, typename... Ss>
    void get_scenes(scene_acceptor<Scenes...>& acceptor) const
    {
        acceptor.single_scene_holder<S>::set(single_scene_holder<S>::get());
        if constexpr(sizeof...(Ss) != 0) get_scenes<Ss...>(acceptor);
    }

    template<typename S, typename... Ss>
    bool has_scenes() const
    {
        if(!single_scene_holder<S>::get()) return false;
        if constexpr(sizeof...(Ss) != 0) return has_scenes<Ss...>();
        return true;
    }
};

template<typename... Scenes>
class composite_scene: public Scenes...
{
public:
    template<typename T>
    void add(T* thing);
    template<typename T>
    void remove(T* thing);
    void clear_all();
    void update_all(duration delta);

    // Glue for composite_scene convenience functions, do not call directly.
    template<typename T>
    void add_impl(T* thing);
    template<typename T>
    void remove_impl(T* thing);
    void clear_impl();
    void update_impl(duration delta);

private:
    template<typename T, typename U, typename=void>
    struct has_add_impl: std::false_type { };

    template<typename T, typename U>
    struct has_add_impl<
        T, U,
        decltype((void) std::declval<T>().add_impl((U*)nullptr), void())
    > : std::true_type { };

    template<typename T, typename S, typename... Rest>
    void add_internal(T* thing, S* base, Rest*... rest);
    template<typename T>
    void add_internal(T* thing);

    template<typename T, typename U, typename=void>
    struct has_remove_impl: std::false_type { };

    template<typename T, typename U>
    struct has_remove_impl<
        T, U,
        decltype((void) std::declval<T>().remove_impl((U*)nullptr), void())
    > : std::true_type { };

    template<typename T, typename S, typename... Rest>
    void remove_internal(T* thing, S* base, Rest*... rest);
    template<typename T>
    void remove_internal(T* thing);

    template<typename T, typename=void>
    struct has_clear_impl: std::false_type { };

    template<typename T>
    struct has_clear_impl<
        T,
        decltype((void) std::declval<T>().clear_impl(), void())
    > : std::true_type { };

    template<typename S, typename... Rest>
    void clear_internal(S* base, Rest*... rest);
    void clear_internal();

    template<typename T, typename=void>
    struct has_update_impl: std::false_type { };

    template<typename T>
    struct has_update_impl<
        T,
        decltype((void) std::declval<T>().update_impl(duration()), void())
    > : std::true_type { };

    template<typename S, typename... Rest>
    void update_internal(duration delta, S* base, Rest*... rest);
    void update_internal(duration delta);
};

using render_scene = composite_scene<
    camera_scene,
    object_scene,
    light_scene,
    shadow_scene,
    environment_scene,
    sg_scene
>;

using scene_2d = composite_scene<
    camera_scene,
    sprite_scene
>;

} // namespace lt

#include "scene.tcc"

#endif
