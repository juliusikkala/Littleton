#include "buffer.hh"
#include <cstring>

buffer::buffer(buffer_type type, const void* data, size_t size)
: buf(0), type(type)
{
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

buffer::buffer(buffer&& other)
: buf(other.buf), type(other.type)
{
    other.buf = 0;
}

buffer::~buffer()
{
    if(buf != 0) glDeleteBuffers(1, &buf);
}

GLuint buffer::get_buffer() const
{
    return buf;
}

buffer::buffer_type buffer::get_type() const
{
    return type;
}
