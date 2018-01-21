#ifndef MODEL_HH
#define MODEL_HH
#include "material.hh"
#include "vertex_buffer.hh"
#include <vector>

struct model
{
public:
    struct vertex_group
    {
        material* mat;
        vertex_buffer* vertex;
    };

    size_t group_count() const;
    void add_vertex_group(
        material* mat,
        vertex_buffer* vertex
    );
    void remove_vertex_group(size_t i);
    const vertex_group& operator[](size_t i) const;

private:
    std::vector<vertex_group> groups;
}; 

#endif
