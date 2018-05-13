#include <algorithm>
#include "resource_pool.hh"

namespace lt
{

template<typename T>
generic_resource_pool<T>::generic_resource_pool(context& ctx)
: glresource(ctx) { }

template<typename T>
generic_resource_pool<T>::~generic_resource_pool() { }

template<typename T>
T* generic_resource_pool<T>::add(
    const std::string& name,
    T* tex,
    bool ignore_duplicate
){
    auto it = resources.find(name);
    if(it != resources.end())
    {
        if(ignore_duplicate)
        {
            delete tex;
            return it->second.get();
        }
        else throw std::runtime_error("Resource " + name + " already exists!");
    }
    resources.emplace(name, tex);
    return tex;
}

template<typename T>
T* generic_resource_pool<T>::add(
    const std::string& name,
    T&& tex,
    bool ignore_duplicate
){
    auto it = resources.find(name);
    if(it != resources.end())
    {
        if(ignore_duplicate) return it->second.get();
        else throw std::runtime_error("Resource " + name + " already exists!");
    }

    T* new_tex = new T(std::move(tex));
    resources.emplace(name, new_tex);
    return new_tex;
}

template<typename T>
void generic_resource_pool<T>::remove(const std::string& name)
{
    resources.erase(name);
}

template<typename T>
const T* generic_resource_pool<T>::get(const std::string& name)
{
    auto it = resources.find(name);
    if(it == resources.end())
        throw std::runtime_error("Unable to get generic_resource " + name);
    return it->second.get();
}

template<typename T>
bool generic_resource_pool<T>::contains(const std::string& name)
{
    auto it = resources.find(name);
    return it != resources.end();
}

template<typename T>
void generic_resource_pool<T>::load_all()
{
    for(auto& pair: resources)
    {
        pair.second->load();
    }
}

template<typename T>
void generic_resource_pool<T>::unload_all()
{
    for(auto& pair: resources)
    {
        pair.second->unload();
    }
}

template<typename T>
typename generic_resource_pool<T>::const_iterator
generic_resource_pool<T>::cbegin() const
{
    return resources.cbegin();
}

template<typename T>
typename generic_resource_pool<T>::const_iterator
generic_resource_pool<T>::cend() const
{
    return resources.cend();
}

} // namespace lt
