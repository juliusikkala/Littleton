#include <algorithm>
#include "resource_pool.hh"
#include <boost/core/demangle.hpp>

namespace lt
{

template<typename T>
const std::string generic_resource_pool<T>::type_string = boost::core::demangle(
    typeid(T).name()
);

template<typename T>
generic_resource_pool<T>::generic_resource_pool(context& ctx)
: glresource(ctx), parent(nullptr) { }

template<typename T>
generic_resource_pool<T>::generic_resource_pool(generic_resource_pool<T>* pool)
: glresource(pool->get_context()), parent(pool) { }

template<typename T>
generic_resource_pool<T>::~generic_resource_pool() { }

template<typename T>
T* generic_resource_pool<T>::add(
    const std::string& name,
    T* tex,
    bool ignore_duplicate
){
    if(!tex)
    {
        throw std::runtime_error(
            "Attempted to add a null pointer as a " + type_string +
            " with name " + name
        );
    }

    auto it = resources.find(name);
    if(it != resources.end())
    {
        if(ignore_duplicate)
        {
            delete tex;
            return it->second.get();
        }
        else throw std::runtime_error(
            type_string + " " + name + " already exists!"
        );
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
        else throw std::runtime_error(
            type_string + " " + name + " already exists!"
        );
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
const T* generic_resource_pool<T>::get(const std::string& name) const
{
    auto it = resources.find(name);
    if(it == resources.end())
    {
        if(parent) return parent->get(name);
        else throw std::runtime_error(
            "Unable to get " + type_string + " " + name
        );
    }
    return it->second.get();
}

template<typename T>
T* generic_resource_pool<T>::get_mutable(const std::string& name)
{
    auto it = resources.find(name);
    if(it == resources.end())
    {
        if(parent) return parent->get_mutable(name);
        else throw std::runtime_error(
            "Unable to get " + type_string + " " + name
        );
    }
    return it->second.get();
}

template<typename T>
bool generic_resource_pool<T>::contains(const std::string& name) const
{
    auto it = resources.find(name);
    if(it == resources.end())
    {
        if(parent) return parent->contains(name);
        else return false;
    }
    return true;
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

template<typename T>
lazy_resource_pool<T>::lazy_resource_pool(context& ctx)
: glresource(ctx), generic_resource_pool<T>(ctx) {}

template<typename T>
lazy_resource_pool<T>::lazy_resource_pool(generic_resource_pool<T>* parent)
: glresource(parent->get_context()), generic_resource_pool<T>(parent) {}

template<typename T>
void lazy_resource_pool<T>::load_all()
{
    for(auto& pair: this->resources)
    {
        pair.second->load();
    }
}

template<typename T>
void lazy_resource_pool<T>::unload_all()
{
    for(auto& pair: this->resources)
    {
        pair.second->unload();
    }
}

} // namespace lt
