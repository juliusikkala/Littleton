#ifndef RESOURCES_HH
#define RESOURCES_HH
#include <unordered_map>
#include <memory>
#include <string>
#include <typeinfo>
#include <typeindex>

class basic_resource_container
{
public:
    basic_resource_container();
    virtual ~basic_resource_container();

    void pin() const;
    void unpin() const;

protected:
    virtual void load() const = 0;
    virtual void unload() const = 0;

private:
    mutable unsigned references;
};

template<typename T>
class resource_container: public basic_resource_container
{
public:
    template<typename... Args>
    resource_container(Args&&... args);

    const T& data() const;

protected:
    void load() const override final;
    void unload() const override final;

private:
    mutable T t;
};

class resource_manager;

template<typename T>
class resource
{
public:
    resource(
        resource_manager& manager,
        const std::string& resource_name
    );
    resource(const resource& res);
    virtual ~resource();

protected:
    const T& data() const;

private:
    const resource_container<T>& data_container;
};

class resource_manager
{
template<typename T>
friend class resource;
public:
    resource_manager();
    resource_manager(const resource_manager& other) = delete;
    resource_manager(resource_manager& other) = delete;
    ~resource_manager();

    template<typename T, typename... Args>
    void create(const std::string& name, Args&&... args);

    void add_dfo(const std::string& dfo_path);

    template<typename T>
    void pin(const std::string& name);

    template<typename T>
    void unpin(const std::string& name);

private:
    template<typename T>
    resource_container<T>& get_container(const std::string& name);

    struct name_type
    {
        std::type_index type;
        std::string name;

        bool operator==(const name_type& other) const;
    };

    class name_type_hash
    {
    public:
        size_t operator()(const name_type& nt) const;
    };

    std::unordered_map<
        name_type,
        std::unique_ptr<basic_resource_container>,
        name_type_hash
    > resources;
};

#include "resources.tcc"
#endif
