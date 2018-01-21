#include "vertex_buffer.hh"
#include <cstring>

static size_t vertex_size(vertex_buffer::vertex_type type)
{
    switch(type)
    {
    case vertex_buffer::VERTEX_PN:
        return 6*sizeof(float);
    case vertex_buffer::VERTEX_PNTT:
        return 12*sizeof(float);
    default:
        return 0;
    }
}

vertex_buffer::vertex_buffer(): vbo(0), ibo(0), vao(0)
{
}

vertex_buffer::vertex_buffer(
    vertex_type type,
    size_t vertex_count,
    const float* vertices,
    size_t index_count,
    const uint32_t* indices
): vbo(0), ibo(0), vao(0)
{
    basic_load(type, vertex_count, vertices, index_count, indices);
}

vertex_buffer::vertex_buffer(vertex_buffer&& other)
{
    other.load();

    vbo = other.vbo;
    ibo = other.ibo;
    vao = other.vao;
    index_count = other.index_count;
    type = other.type;

    other.vbo = 0;
    other.ibo = 0;
    other.vao = 0;
}

vertex_buffer::~vertex_buffer()
{
    basic_unload();
}

GLuint vertex_buffer::get_vbo() const { load(); return vbo; }
GLuint vertex_buffer::get_ibo() const { load(); return ibo; }
GLuint vertex_buffer::get_vao() const { load(); return vao; }
void vertex_buffer::draw() const
{
    load();
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, index_count);
}

vertex_buffer::vertex_type vertex_buffer::get_type() const
{
    load();
    return type;
}

void vertex_buffer::basic_load(
    vertex_type type,
    size_t vertex_count,
    const float* vertices,
    size_t index_count,
    const uint32_t* indices
) const {
    if(vao) return;

    this->type = type;
    this->index_count = index_count;

    size_t vs = vertex_size(type);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertex_count * vs,
        vertices,
        GL_STATIC_DRAW
    );

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        index_count * sizeof(uint32_t),
        indices,
        GL_STATIC_DRAW
    );

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vs, 0);

    // Normal
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        vs,
        (const GLvoid*)(3*sizeof(float))
    );

    if(type == VERTEX_PNTT)
    {
        // Tangent
        glVertexAttribPointer(
            2,
            4,
            GL_FLOAT,
            GL_FALSE,
            vs,
            (const GLvoid*)(6*sizeof(float))
        );

        // Texture
        glVertexAttribPointer(
            3,
            2,
            GL_FLOAT,
            GL_FALSE,
            vs,
            (const GLvoid*)(10*sizeof(float))
        );
    }
}

void vertex_buffer::basic_unload() const
{
    if(vbo != 0)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if(ibo != 0)
    {
        glDeleteBuffers(1, &ibo);
        ibo = 0;
    }
    if(vao != 0)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
}
