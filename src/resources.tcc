template<typename T>
template<typename... Args>
resource_container<T>::resource_container(Args&&... args)
: t(std::forward<Args>(args)...)
{
}

template<typename T>
const T& resource_container<T>::data() const
{
    return t;
}

template<typename T>
void resource_container<T>::load() const
{
    t.load();
}

template<typename T>
void resource_container<T>::unload() const
{
    t.unload();
}

template<typename T, typename... Args>
void resource_manager::create(const std::string& name, Args&&... args)
{
    name_type key = {typeid(T), name};
    auto it = resources.find(key);
    if(it != resources.end())
    {
        throw std::runtime_error("Resource " + name + " already exists!");
    }

    resources[key] = new resource_container<typename T::data_type>(
        std::forward<Args>(args)...
    );
}

template<typename T>
resource_container<T>& resource_manager::get_container(const std::string& name)
{
    auto it = resources.find({typeid(T), name});
    if(it == resources.end())
    {
        throw std::runtime_error("Unable to find resource " + name);
    }
    resource_container<T>* container =
        dynamic_cast<resource_container<T>*>(it->second.get());

    if(container == nullptr)
    {
        throw std::runtime_error("Invalid type for resource " + name);
    }
    return *container;
}

template<typename T>
void resource_manager::pin(const std::string& name)
{
    auto it = resources.find({typeid(T), name});
    if(it == resources.end())
    {
        throw std::runtime_error("Unable to find resource " + name);
    }
    it->second->pin();
}

template<typename T>
void resource_manager::unpin(const std::string& name)
{
    auto it = resources.find({typeid(T), name});
    if(it == resources.end())
    {
        throw std::runtime_error("Unable to find resource " + name);
    }
    it->second->unpin();
}

template<typename T>
resource<T>::resource(
    resource_manager& manager,
    const std::string& resource_name
): data_container(manager.get_container<T>(resource_name))
{
    data_container.pin();
}

template<typename T>
resource<T>::resource(const resource& res): data_container(res.data_container)
{
    data_container.pin();
}

template<typename T>
resource<T>::~resource()
{
    data_container.unpin();
}

template<typename T>
const T& resource<T>::data() const
{
    return data_container.data();
}
