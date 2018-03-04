#ifndef MODEL_HH
#define MODEL_HH
#include <vector>

class material;
class vertex_buffer;
class model
{
public:
    struct vertex_group
    {
        material* mat;
        vertex_buffer* mesh;
    };

    size_t group_count() const;
    void add_vertex_group(
        material* mat,
        vertex_buffer* mesh
    );
    void remove_vertex_group(size_t i);
    const vertex_group& operator[](size_t i) const;

    using iterator = std::vector<vertex_group>::iterator;
    using const_iterator = std::vector<vertex_group>::const_iterator;

    iterator begin();
    const_iterator cbegin() const;

    iterator end();
    const_iterator cend() const;

private:
    std::vector<vertex_group> groups;
}; 

#endif
