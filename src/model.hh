#ifndef MODEL_HH
#define MODEL_HH
#include <vector>
#include <cstddef>

class material;
class primitive;
class model
{
public:
    struct vertex_group
    {
        const material* mat;
        const primitive* mesh;
    };

    size_t group_count() const;
    void add_vertex_group(
        const material* mat,
        const primitive* mesh
    );
    void remove_vertex_group(size_t i);
    const vertex_group& operator[](size_t i) const;

    using iterator = std::vector<vertex_group>::iterator;
    using const_iterator = std::vector<vertex_group>::const_iterator;

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;

    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

private:
    std::vector<vertex_group> groups;
}; 

#endif
