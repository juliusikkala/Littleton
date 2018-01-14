template<typename T>
resource_ptr<T>::resource_ptr() {}

template<typename T>
resource_ptr<T>::resource_ptr(
    std::function<void*()> create_resource,
    std::function<void(void*)> delete_resource
): basic_resource_ptr(
    new shared {
        0, 0,
        create_resource, delete_resource,
        nullptr
    }
) { }

template<typename T>
resource_ptr<T>::resource_ptr(const resource_ptr<T>& other)
: basic_resource_ptr(other) { }

template<typename T>
resource_ptr<T>::resource_ptr(resource_ptr<T>&& other)
: basic_resource_ptr(std::move(other)) {}

template<typename T>
resource_ptr<T>::resource_ptr(T* ptr)
: basic_resource_ptr(
    ptr ?
        new shared {
            0, 1,
            [=]() { return nullptr; },
            [](void* ptr){ delete ((T*)ptr); },
            ptr
        } : nullptr
) { }

template<typename T>
resource_ptr<T>::resource_ptr(shared* s)
: basic_resource_ptr(s) {}

template<typename T>
template<typename... Args>
resource_ptr<T> resource_ptr<T>::create(Args&&... args)
{
    return resource_ptr<T>(
        [=](){ return new T(args...); },
        [](void* ptr){ delete ((T*)ptr); }
    );
}

template<typename T>
resource_ptr<T> resource_ptr<T>::ref(T& reference)
{
    return resource_ptr<T>(new shared {
        0, 1,
        [refptr = &reference](){ return refptr; },
        [](void *ptr){},
        &reference
    });
}

template<typename T>
T& resource_ptr<T>::operator*() const
{
    if(local_pins == 0) pin();
    return *((T*)s->resource);
}

template<typename T>
T* resource_ptr<T>::operator->() const
{
    if(local_pins == 0) pin();
    return (T*)s->resource;
}

template<typename T>
T* resource_ptr<T>::get() const
{
    if(local_pins == 0) pin();
    return (T*)s->resource;
}

template<typename T>
resource_ptr<T>& resource_ptr<T>::operator=(T* ptr)
{
    reset(
        ptr ? new shared {
            0, 1,
            [=]() { return nullptr; },
            [](void* ptr){ delete ((T*)ptr); },
            ptr
        } : nullptr
    );
    return *this;
}

template<typename T>
resource_ptr<T>& resource_ptr<T>::operator=(resource_ptr<T>&& other)
{
    reset(other.s);
    other.reset(nullptr);
    return *this;
}

template<typename T>
resource_ptr<T>& resource_ptr<T>::operator=(const resource_ptr<T>& other)
{
    reset(other.s);
    return *this;
}

template<typename T>
resource_ptr<T>::resource_ptr(const basic_resource_ptr& other)
: basic_resource_ptr(other) {}

template<typename T, typename... Args>
const resource_ptr<T>& resource_store::create(
    const std::string& name,
    Args&&... args
){
    name_type key = {typeid(T), name};
    auto it = resources.find(key);
    if(it != resources.end())
    {
        throw std::runtime_error("Resource " + name + " already exists!");
    }

    basic_resource_ptr& ptr = (resources[key] = resource_ptr<T>::create(
        std::forward<Args>(args)...)
    );

    return *static_cast<resource_ptr<T>*>(&ptr);
}

template<typename T>
const resource_ptr<T>& resource_store::add(
    const std::string& name,
    resource_ptr<T> res
){
    name_type key = {typeid(T), name};
    auto it = resources.find(key);
    if(it != resources.end())
    {
        throw std::runtime_error("Resource " + name + " already exists!");
    }

    basic_resource_ptr& ptr = (resources[key] = res);

    return *static_cast<resource_ptr<T>*>(&ptr);
}

template<typename T>
void resource_store::remove(const std::string& name)
{
    name_type key = {typeid(T), name};
    resources.erase(key);
}

template<typename T>
const resource_ptr<T>& resource_store::get(const std::string& name) const
{
    auto it = resources.find({typeid(T), name});
    if(it == resources.end())
    {
        throw std::runtime_error("Unable to find resource " + name);
    }

    return *static_cast<const resource_ptr<T>*>(&(it->second));
}

template<typename T>
void resource_store::pin(const std::string& name)
{
    auto it = resources.find({typeid(T), name});
    if(it == resources.end())
    {
        throw std::runtime_error("Unable to find resource " + name);
    }
    it->second.pin();
}

template<typename T>
void resource_store::unpin(const std::string& name)
{
    auto it = resources.find({typeid(T), name});
    if(it == resources.end())
    {
        throw std::runtime_error("Unable to find resource " + name);
    }
    it->second.unpin();
}
