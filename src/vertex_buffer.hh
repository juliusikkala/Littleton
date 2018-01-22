#ifndef VERTEX_BUFFER_HH
#define VERTEX_BUFFER_HH
#include <memory>
#include "glheaders.hh"
#include "resources.hh"

class vertex_buffer: public resource
{
public:
    enum vertex_type
    {
        VERTEX_PN = 0, // Position, Normal
        VERTEX_PNTT = 1, // Position, Normal, Tangent, Texture
        VERTEX_P = 2 // Position
    };

    vertex_buffer();
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

    /* Note that the created buffer is _not_ lazily loaded! */
    static vertex_buffer* create_fullscreen();

protected:
    void basic_load(
        vertex_type type,
        size_t vertex_count,
        const float* vertices,
        size_t index_count,
        const uint32_t* indices
    ) const;

    void basic_unload() const;

    mutable GLuint vbo, ibo, vao;
    mutable size_t index_count;
    mutable vertex_type type;
};

#endif
