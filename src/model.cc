#include "model.hh"

size_t model::group_count() const
{
    return groups.size();
}

void model::add_vertex_group(
    material* mat,
    vertex_buffer* mesh
){
    groups.emplace_back(vertex_group{mat, mesh});
}

void model::remove_vertex_group(size_t i)
{
    groups.erase(groups.begin() + i);
}

const model::vertex_group& model::operator[](size_t i) const
{
    return groups[i];
}

model::iterator model::begin()
{
    return groups.begin();
}

model::const_iterator model::cbegin() const
{
    return groups.cbegin();
}

model::iterator model::end()
{
    return groups.end();
}

model::const_iterator model::cend() const
{
    return groups.cend();
}
