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
T resource_manager::create(const std::string& name, Args&&... args)
{
    name_type key = {typeid(T), name};
    auto it = resources.find(key);
    if(it != resources.end())
    {
        throw std::runtime_error("Resource " + name + " already exists!");
    }

    resources[key].reset(new resource_container<typename T::data_type>(
        std::forward<Args>(args)...
    ));
    return T(std::dynamic_pointer_cast<
        resource_container<typename T::data_type>
    >(resources[key]));
}

template<typename T>
std::shared_ptr<resource_container<T>> resource_manager::get_container(const std::string& name)
{
    auto it = resources.find({typeid(T), name});
    if(it == resources.end())
    {
        throw std::runtime_error("Unable to find resource " + name);
    }
    std::shared_ptr<resource_container<T>> container =
        std::dynamic_pointer_cast<resource_container<T>>(it->second);

    if(!container)
    {
        throw std::runtime_error("Invalid type for resource " + name);
    }
    return container;
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
): pins(0), data_container(manager.get_container<T>(resource_name)) {}

template<typename T>
resource<T>::resource(
    std::shared_ptr<resource_container<T>> data_container
): pins(0), data_container(data_container) {}

template<typename T>
resource<T>::resource(
    const resource& res
): pins(0), data_container(res.data_container) {}

template<typename T>
resource<T>::~resource()
{
    while(pins--) data_container->unpin();
}

template<typename T>
void resource<T>::pin() const
{
    pins++;
    data_container->pin();
}

template<typename T>
void resource<T>::unpin() const
{
    if(pins != 0)
    {
        pins--;
        data_container->unpin();
    }
}

template<typename T>
const T& resource<T>::data() const
{
    if(pins == 0) pin();
    return data_container->data();
}
