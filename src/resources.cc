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

bool resource_manager::name_type::operator==(const name_type& other) const
{
    return other.type == type && other.name == name;
}

size_t resource_manager::name_type_hash::operator()(const name_type& nt) const
{
    return nt.type.hash_code() + std::hash<std::string>()(nt.name);
}
