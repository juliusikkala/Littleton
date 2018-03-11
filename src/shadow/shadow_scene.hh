#ifndef SHADOW_SCENE_HH
#define SHADOW_SCENE_HH
#include "shadow_map.hh"
#include <memory>
#include <map>
#include <set>

class shadow_scene
{
public:
    using directional_shadow_map_map = std::map<
        std::unique_ptr<directional_shadow_map_impl>,
        std::vector<directional_shadow_map*>
    >;

    shadow_scene();
    ~shadow_scene();

    void add_shadow_map(directional_shadow_map* sm, resource_pool& pool);
    void remove_shadow_map(directional_shadow_map* sm);

    void clear_shadow_maps();

    size_t shadow_map_count() const;

    const directional_shadow_map_map& get_directional_shadow_maps() const;

private:
    directional_shadow_map_map directional_shadow_maps;
};

#endif
