#ifndef BUFFER_HH
#define BUFFER_HH
#include <memory>
#include "glheaders.hh"
#include "resources.hh"

class buffer
{
public:
    enum buffer_type
    {
        VERTEX_PN = 0, // Position, Normal
        VERTEX_PNT = 1, // Position, Normal, Texture
        INDEX32 = 2 // 32-bit indices
    };

    buffer(buffer_type type, const void* buf, size_t size);
    buffer(const buffer& other) = delete;
    buffer(buffer&& other);
    ~buffer();

    GLuint get_buffer() const;
    buffer_type get_type() const;
private:
    GLuint buf;
    buffer_type type;
};

using buffer_ptr = resource_ptr<buffer>;

#endif
