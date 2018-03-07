#include <algorithm>

template<typename T>
resource_pool::typed_container<T>::typed_container(T* data)
{
    this->data = data;
}

template<typename T>
resource_pool::typed_container<T>::~typed_container() { delete (T*)data; }

template<typename T, typename I>
void resource_pool::resource_iterator<T, I>::swap(
    resource_iterator<T, I>& other
) noexcept
{
    std::swap(it, other.it);
}

template<typename T, typename I>
resource_pool::resource_iterator<T, I>&
resource_pool::resource_iterator<T, I>::operator++()
{
    it++;
    return *this;
}

template<typename T, typename I>
resource_pool::resource_iterator<T, I>
resource_pool::resource_iterator<T, I>::operator++(int)
{
    resource_iterator<T, I> tmp(*this);
    it++;
    return tmp;
}

template<typename T, typename I>
template<typename U>
bool resource_pool::resource_iterator<T, I>::operator==(
    const resource_iterator<U, I>& other
) const
{
    return this->it == other.it;
}

template<typename T, typename I>
template<typename U>
bool resource_pool::resource_iterator<T, I>::operator!=(
    const resource_iterator<U, I>& other
) const
{
    return this->it != other.it;
}

template<typename T, typename I>
T* resource_pool::resource_iterator<T, I>::operator*() const
{
    return (T*)it->second->data;
}

template<typename T, typename I>
const std::string& resource_pool::resource_iterator<T, I>::name() const
{
    return it->first;
}

template<typename T, typename I>
template<typename J>
resource_pool::resource_iterator<T, I>::operator
resource_iterator<const T, J>() const
{
    return resource_iterator<const T, J>(it);
}

template<typename T, typename I>
resource_pool::resource_iterator<T, I>::resource_iterator(I it)
: it(it) { }

template<typename T>
T* resource_pool::add(const std::string& name, T* res)
{
    auto& m = resources[typeid(T)];
    auto it = m.find(name);
    if(it != m.end())
        throw std::runtime_error("Resource " + name + " already exists!");

    m[name].reset(new typed_container<T>(res));
    return res;
}

template<typename T>
T* resource_pool::add(const std::string& name, T&& res)
{
    auto& m = resources[typeid(T)];
    auto it = m.find(name);
    if(it != m.end())
        throw std::runtime_error("Resource " + name + " already exists!");

    T* ptr = new T(std::move(res));
    m[name].reset(new typed_container<T>(ptr));
    return ptr;
}

template<typename T>
bool resource_pool::contains(const std::string& name) const
{
    auto& m = resources[typeid(T)];
    auto it = m.find(name);
    return it != m.end();
}

template<typename T>
void resource_pool::remove(const std::string& name)
{
    resources[typeid(T)].erase(name);
}

template<typename T>
T* resource_pool::get(const std::string& name) const
{
    auto it = resources.find(typeid(T));
    if(it == resources.end())
    {
        throw std::runtime_error("Unable to find resource " + name);
    }
    auto it2 = it->second.find(name);
    if(it2 == it->second.end())
    {
        throw std::runtime_error("Unable to find resource " + name);
    }

    return static_cast<T*>(it2->second->data);
}

template<typename T>
size_t resource_pool::size() const
{
    return resources[typeid(T)].size();
}

template<typename T>
resource_pool::iterator<T> resource_pool::begin()
{
    return iterator<T>(resources[typeid(T)].begin());
}

template<typename T>
resource_pool::iterator<T> resource_pool::end()
{
    return iterator<T>(resources[typeid(T)].end());
}

template<typename T>
resource_pool::const_iterator<T> resource_pool::cbegin() const
{
    return const_iterator<T>(resources[typeid(T)].cbegin());
}

template<typename T>
resource_pool::const_iterator<T> resource_pool::cend() const
{
    return const_iterator<T>(resources[typeid(T)].cend());
}

template<typename T>
resource_pool::iterator<T> resource_pool::iterable<T>::begin()
{
    return resource_pool::iterator<T>(m.begin());
}

template<typename T>
resource_pool::iterator<T> resource_pool::iterable<T>::end()
{
    return resource_pool::iterator<T>(m.end());
}

template<typename T>
resource_pool::iterable<T>::iterable(inner_map& m): m(m) {}

template<typename T>
resource_pool::const_iterator<T>
resource_pool::const_iterable<T>::cbegin() const
{
    return resource_pool::const_iterator<T>(m.cbegin());
}

template<typename T>
resource_pool::const_iterator<T>
resource_pool::const_iterable<T>::cend() const
{
    return resource_pool::const_iterator<T>(m.cend());
}

template<typename T>
resource_pool::const_iterable<T>::const_iterable(
    const inner_map& m
): m(m) {}

template<typename T>
resource_pool::iterable<T> resource_pool::get_iterable()
{
    return iterable<T>(resources[typeid(T)]);
}

template<typename T>
resource_pool::const_iterable<T> resource_pool::get_const_iterable() const
{
    return const_iterable<T>(resources[typeid(T)]);
}
