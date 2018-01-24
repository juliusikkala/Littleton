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
    case vertex_buffer::VERTEX_P:
        return 3*sizeof(float);
    default:
        return 0;
    }
}

static size_t position_size(vertex_buffer::vertex_type type)
{
    switch(type)
    {
    default:
        return 3;
    }
}

static size_t normal_size(vertex_buffer::vertex_type type)
{
    switch(type)
    {
    case vertex_buffer::VERTEX_PN:
        return 3;
    case vertex_buffer::VERTEX_PNTT:
        return 3;
    default:
        return 0;
    }
}

static size_t tangent_size(vertex_buffer::vertex_type type)
{
    switch(type)
    {
    case vertex_buffer::VERTEX_PNTT:
        return 4;
    default:
        return 0;
    }
}

static size_t uv_size(vertex_buffer::vertex_type type)
{
    switch(type)
    {
    case vertex_buffer::VERTEX_PNTT:
        return 2;
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

shader::definition_map vertex_buffer::get_definitions() const
{
    load();
    switch(type)
    {
    case vertex_buffer::VERTEX_PN:
        return {{"VERTEX_POSITION", "0"}, {"VERTEX_NORMAL", "1"}};
    case vertex_buffer::VERTEX_PNTT:
        return {
            {"VERTEX_POSITION", "0"},
            {"VERTEX_NORMAL", "1"},
            {"VERTEX_TANGENT", "2"},
            {"VERTEX_UV", "3"},
        };
    case vertex_buffer::VERTEX_P:
        return {
            {"VERTEX_POSITION", "0"}
        };
    default:
        return {};
    }
}

GLuint vertex_buffer::get_vbo() const { load(); return vbo; }
GLuint vertex_buffer::get_ibo() const { load(); return ibo; }
GLuint vertex_buffer::get_vao() const { load(); return vao; }

void vertex_buffer::draw() const
{
    load();
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);
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
    size_t sizes[4] = {
        position_size(type),
        normal_size(type),
        tangent_size(type),
        uv_size(type)
    };
    size_t offset = 0;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * vs, vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        index_count * sizeof(uint32_t),
        indices,
        GL_STATIC_DRAW
    );

    for(unsigned i = 0; i < sizeof(sizes)/sizeof(size_t); ++i)
    {
        if(sizes[i])
        {
            glVertexAttribPointer(
                i,
                sizes[i],
                GL_FLOAT,
                GL_FALSE,
                vs,
                (const GLvoid*)(offset)
            );
            offset += sizes[i]*sizeof(float);
            glEnableVertexAttribArray(i);
        } else glDisableVertexAttribArray(i);
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

static const GLfloat quad_vertices[] = {
    1.0f, 1.0f, 0,
    1.0f, -1.0f, 0,
    -1.0f, 1.0f, 0,
    -1.0f, -1.0f, 0
};

static const GLuint quad_indices[] = {0,2,1,1,2,3};

vertex_buffer vertex_buffer::create_fullscreen()
{
    return vertex_buffer(
        VERTEX_P,
        sizeof(quad_vertices)/(sizeof(float)*3),
        quad_vertices,
        sizeof(quad_indices)/sizeof(GLuint),
        quad_indices
    );
}
