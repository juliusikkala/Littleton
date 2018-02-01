#ifndef VERTEX_BUFFER_HH
#define VERTEX_BUFFER_HH
#include <memory>
#include "glheaders.hh"
#include "resources.hh"
#include "shader.hh"

class vertex_buffer: public resource, public glresource
{
public:
    enum vertex_type
    {
        VERTEX_PN = 0, // Position, Normal
        VERTEX_PNTT = 1, // Position, Normal, Tangent, Texture
        VERTEX_P = 2, // Position
        VERTEX_PT = 3 // Position, Texture
    };

    vertex_buffer(context& ctx);
    vertex_buffer(
        context& ctx,
        vertex_type type,
        size_t vertex_count,
        const float* vertices,
        size_t index_count,
        const uint32_t* indices
    );
    vertex_buffer(vertex_buffer&& other);
    ~vertex_buffer();

    shader::definition_map get_definitions() const;

    GLuint get_vbo() const;
    GLuint get_ibo() const;
    GLuint get_vao() const;
    void draw() const;
    vertex_type get_type() const;

    // Creates a square buffer with coordinates at (1,1), (1,-1), (-1,1) and
    // (-1,-1).
    // Note that the created buffer is _not_ lazily loaded!
    static vertex_buffer create_square(context& ctx);

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
