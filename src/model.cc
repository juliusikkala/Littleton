#include "model.hh"

size_t model::group_count() const
{
    return groups.size();
}

void model::add_vertex_group(
    const material_ptr& mat,
    const buffer_ptr& vertex,
    const buffer_ptr& index
){
    groups.emplace_back(vertex_group{mat, vertex, index});
}

void model::remove_vertex_group(size_t i)
{
    groups.erase(groups.begin() + i);
}

const model::vertex_group& model::operator[](size_t i) const
{
    return groups[i];
}

