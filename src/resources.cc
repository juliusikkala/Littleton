#include "resources.hh"
#include <stdexcept>

basic_resource_container::basic_resource_container(): references(0) { }
basic_resource_container::~basic_resource_container()
{ }

void basic_resource_container::pin() const
{
    references++;
    if(references == 1)
    {
        load();
    }
}

void basic_resource_container::unpin() const
{
    if(references == 1)
    {
        unload();
    }
    else if (references != 0)
    {
        references--;
    }
    else throw std::runtime_error("Too many unpins!");
}

resource_manager::resource_manager() {}

resource_manager::~resource_manager() {}

void resource_manager::add_dfo(const std::string& dfo_path)
{
    // TODO: Implement
}

void resource_manager::pin(const std::string& name)
{
    auto it = resources.find(name);
    if(it == resources.end())
    {
        throw std::runtime_error("Unable to find resource " + name);
    }
    it->second->pin();
}

void resource_manager::unpin(const std::string& name)
{
    auto it = resources.find(name);
    if(it == resources.end())
    {
        throw std::runtime_error("Unable to find resource " + name);
    }
    it->second->unpin();
}

