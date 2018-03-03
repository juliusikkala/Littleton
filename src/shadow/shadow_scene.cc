#include "shadow_scene.hh"
#include "shadow_map.hh"

shadow_scene::shadow_scene(std::set<basic_shadow_map*> shadow_maps)
{
    set_shadow_maps(shadow_maps);
}

shadow_scene::~shadow_scene()
{
}

void shadow_scene::add_shadow_map(basic_shadow_map* sm)
{
    ((const shadow_scene*)this)->add_shadow_map(sm);
}

void shadow_scene::remove_shadow_map(basic_shadow_map* sm)
{
    ((const shadow_scene*)this)->remove_shadow_map(sm);
}

void shadow_scene::clear_shadow_maps()
{
    shadow_maps.clear();
}

size_t shadow_scene::shadow_map_count() const
{
    return shadow_maps.size();
}

void shadow_scene::set_shadow_maps(
    const std::set<basic_shadow_map*>& shadow_maps
){
    // Remove old entries without removing implementations
    for(auto& pair: this->shadow_maps)
    {
        pair.second.clear();
    }

    // Add new entries
    for(basic_shadow_map* sm: shadow_maps)
    {
        add_shadow_map(sm);
    }
}

const shadow_scene::shadow_map_map& shadow_scene::get_shadow_maps() const
{
    return shadow_maps;
}

std::map<light*, basic_shadow_map*>
shadow_scene::get_shadow_maps_by_light() const
{
    std::map<light*, basic_shadow_map*> by_light;
    for(auto& pair: this->shadow_maps)
    {
        for(basic_shadow_map* sm: pair.second)
        {
            by_light[sm->get_light()] = sm;
        }
    }
    return by_light;
}

void shadow_scene::add_shadow_map(basic_shadow_map* sm) const
{
    for(auto& pair: shadow_maps)
    {
        if(sm->impl_is_compatible(pair.first.get()))
        {
            pair.second.insert(sm);
            return;
        }
    }

    shadow_maps.emplace(sm->create_impl(), std::set<basic_shadow_map*>{sm});
}

void shadow_scene::remove_shadow_map(basic_shadow_map* sm) const
{
    for(auto it = shadow_maps.begin(); it != shadow_maps.end(); ++it)
    {
        auto sm_it = it->second.find(sm);
        if(sm_it != it->second.end())
        {
            it->second.erase(sm_it);
            shadow_maps.erase(it);
            break;
        }
    }
}
