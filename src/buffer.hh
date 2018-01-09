#ifndef BUFFER_HH
#define BUFFER_HH
#include <memory>
#include "glheaders.hh"
#include "resources.hh"

class buffer_data_reader
{
public: 
    virtual ~buffer_data_reader();

    virtual size_t size() = 0;

    // Must return a buffer of size 'size'. Once the data has been copied to
    // the OpenGL buffer, it will be returned to return_data which can the
    // free it if necessary.
    virtual void* loan_data() = 0;
    virtual void return_data(void* data) = 0;
};


class buffer_data
{
public:
    enum buffer_type
    {
        VERTEX_PN = 0, // Position, Normal
        VERTEX_PNT = 1, // Position, Normal, Texture
        INDEX32 = 2 // 32-bit indices
    };

    buffer_data(buffer_type type, const void* buf, size_t size);
    buffer_data(buffer_type type, std::unique_ptr<buffer_data_reader>&& reader);
    buffer_data(const buffer_data& other) = delete;
    ~buffer_data();

    void load();
    void unload();

    GLint buffer;
    buffer_type type;

private:
    std::unique_ptr<buffer_data_reader> reader;
};

class buffer: public resource<buffer_data>
{
public:
    buffer(
        resource_manager& manager,
        const std::string& resource_name
    );

private:
};

#endif
