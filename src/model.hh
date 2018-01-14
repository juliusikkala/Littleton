#ifndef MODEL_HH
#define MODEL_HH
#include "resources.hh"
#include "material.hh"
#include "buffer.hh"

struct model
{
public:
    struct vertex_group
    {
        material_ptr mat;
        buffer_ptr vertex;
        buffer_ptr index;
    };

    size_t group_count() const;
    void add_vertex_group(
        material_ptr mat,
        buffer_ptr vertex,
        buffer_ptr index
    );
    void remove_vertex_group(size_t i);
    const vertex_group& operator[](size_t i) const;

private:
    std::vector<vertex_group> groups;
}; 

using model_ptr = resource_ptr<model>;

#endif
