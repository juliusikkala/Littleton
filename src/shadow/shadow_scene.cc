#include "shadow_scene.hh"
#include "shadow_map.hh"
#include "helpers.hh"

shadow_scene::shadow_scene() {}

shadow_scene::~shadow_scene() {}

void shadow_scene::add_shadow_map(
    directional_shadow_map* sm,
    resource_pool& pool
){
    for(auto& pair: directional_shadow_maps)
    {
        if(sm->impl_is_compatible(pair.first.get()))
        {
            sorted_insert(pair.second, sm);
            return;
        }
    }

    directional_shadow_maps.emplace(
        sm->create_impl(pool),
        std::vector<directional_shadow_map*>{sm}
    );
}

void shadow_scene::remove_shadow_map(directional_shadow_map* sm)
{
    for(auto it = directional_shadow_maps.begin();
        it != directional_shadow_maps.end();
        ++it)
    {
        if(sorted_erase(it->second, sm))
        {
            break;
        }
    }
}

void shadow_scene::clear_shadow_maps()
{
    directional_shadow_maps.clear();
}

size_t shadow_scene::shadow_map_count() const
{
    return directional_shadow_maps.size();
}

const shadow_scene::directional_shadow_map_map&
shadow_scene::get_directional_shadow_maps() const
{
    return directional_shadow_maps;
}

