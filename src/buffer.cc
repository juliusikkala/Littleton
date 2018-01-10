#include "buffer.hh"
#include <cstring>

buffer_data_reader::~buffer_data_reader() {}

class memory_reader: public buffer_data_reader
{
public:
    memory_reader(const void* buf, size_t sz)
    : buf(new char[sz]), sz(sz)
    {
        memcpy(this->buf, buf, sz);
    }

    ~memory_reader()
    {
        delete [] buf;
    }

    size_t size() override
    {
        return sz;
    }

    void* loan_data() override
    {
        return buf;
    }

    void return_data(void* data) override { (void)data; }

private:
    char* buf;
    size_t sz;
};

buffer_data::buffer_data(buffer_type type, const void* buf, size_t size)
: buffer(0), type(type), reader(new memory_reader(buf, size))
{
}

buffer_data::buffer_data(
    buffer_type type,
    std::unique_ptr<buffer_data_reader>&& reader
): buffer(0), type(type), reader(std::move(reader))
{
}

buffer_data::~buffer_data()
{
    unload();
}

void buffer_data::load()
{
    if(buffer == 0)
    {
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        void* data = reader->loan_data();
        glBufferData(GL_ARRAY_BUFFER, reader->size(), data, GL_STATIC_DRAW);
        reader->return_data(data);
    }
}

void buffer_data::unload()
{
    if(buffer != 0)
    {
        glDeleteBuffers(1, &buffer);
        buffer = 0;
    }
}
