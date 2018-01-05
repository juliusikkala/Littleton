#ifndef RESOURCES_HH
#define RESOURCES_HH
#include <map>
#include <memory>
#include <string>

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

    void pin(const std::string& name);
    void unpin(const std::string& name);

private:
    template<typename T>
    resource_container<T>& get_container(const std::string& name);

    std::map<std::string, std::unique_ptr<basic_resource_container>> resources;
};

#include "resources.tcc"
#endif
