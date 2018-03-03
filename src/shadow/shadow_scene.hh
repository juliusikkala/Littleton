#ifndef SHADOW_SCENE_HH
#define SHADOW_SCENE_HH
#include "shadow_map.hh"
#include <memory>
#include <map>
#include <set>

class shadow_scene
{
public:
    using shadow_map_map = std::map<
        std::unique_ptr<shadow_map_impl>,
        std::set<basic_shadow_map*>
    >;

    shadow_scene(std::set<basic_shadow_map*> shadow_maps = {});
    ~shadow_scene();

    void add_shadow_map(basic_shadow_map* sm);
    void remove_shadow_map(basic_shadow_map* sm);

    void clear_shadow_maps();

    size_t shadow_map_count() const;

    void set_shadow_maps(const std::set<basic_shadow_map*>& shadow_maps);

    const shadow_map_map& get_shadow_maps() const;
    std::map<light*, basic_shadow_map*> get_shadow_maps_by_light() const;

private:
    void add_shadow_map(basic_shadow_map* sm) const;
    void remove_shadow_map(basic_shadow_map* sm) const;

    mutable shadow_map_map shadow_maps;
};

#endif
