#include "model.hh"

size_t model::group_count() const
{
    return groups.size();
}

void model::add_vertex_group(
    material_ptr mat,
    buffer_ptr vertex,
    buffer_ptr index
){
    groups.emplace_back(vertex_group{
        std::move(mat),
        std::move(vertex),
        std::move(index)
    });
}

void model::remove_vertex_group(size_t i)
{
    groups.erase(groups.begin() + i);
}

const model::vertex_group& model::operator[](size_t i) const
{
    return groups[i];
}

