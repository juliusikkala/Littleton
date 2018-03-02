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
    // Remove old entries without removing reusable shared resources
    for(auto& pair: this->shadow_maps)
    {
        pair.second.clear();
    }

    // Add new entries
    for(basic_shadow_map* sm: shadow_maps)
    {
        add_shadow_map(sm);
    }

    // Remove shared resources without users
    for(auto it = this->shadow_maps.begin(); it != this->shadow_maps.end();)
    {
        if(it->second.empty()) this->shadow_maps.erase(it++);
        else ++it;
    }
}

const shadow_scene::shadow_map_map& shadow_scene::get_shadow_maps() const
{
    // The shadow map may have changed in a way that requires changes to the
    // shared_resources object, so update them if necessary.
    recalc();
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
        if(sm->merge_shared_resources(pair.first.get()))
        {
            pair.second.insert(sm);
            return;
        }
    }
    shadow_maps.emplace(
        sm->create_shared_resources(),
        std::set<basic_shadow_map*>{sm}
    );
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

void shadow_scene::recalc() const
{
    for(auto it = shadow_maps.begin(); it != shadow_maps.end();)
    {
        bool compatible = true;

        for(basic_shadow_map* sm: it->second)
        {
            if(!sm->merge_shared_resources(it->first.get()))
            {
                compatible = false;
                remove_shadow_map(sm);
                add_shadow_map(sm);
                break;
            }
        }

        if(compatible) it++;
        else it = shadow_maps.begin();
    }
}
