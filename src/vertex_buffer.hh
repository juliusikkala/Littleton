#ifndef VERTEX_BUFFER_HH
#define VERTEX_BUFFER_HH
#include <memory>
#include "glheaders.hh"
#include "resources.hh"

class vertex_buffer
{
public:
    enum vertex_type
    {
        VERTEX_PN = 0, // Position, Normal
        VERTEX_PNTT = 1 // Position, Normal, Tangent, Texture
    };

    vertex_buffer(
        vertex_type type,
        size_t vertex_count,
        const float* vertices,
        size_t index_count,
        const uint32_t* indices
    );
    vertex_buffer(const vertex_buffer& other) = delete;
    vertex_buffer(vertex_buffer&& other);
    ~vertex_buffer();

    GLuint get_vbo() const;
    GLuint get_ibo() const;
    GLuint get_vao() const;
    void draw() const;
    vertex_type get_type() const;

private:
    GLuint vbo, ibo, vao;
    size_t index_count;
    vertex_type type;
};

using vertex_buffer_ptr = resource_ptr<vertex_buffer>;

#endif
