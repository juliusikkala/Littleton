#include "resources.hh"
#include <stdexcept>
#include <memory>
#include "dfo.h"
#include "buffer.hh"

basic_resource_container::basic_resource_container(): references(0) { }
basic_resource_container::~basic_resource_container()
{ }

void basic_resource_container::pin() const
{
    references++;
    if(references == 1)
    {
        load();
    }
}

void basic_resource_container::unpin() const
{
    if(references == 1)
    {
        unload();
    }
    else if (references != 0)
    {
        references--;
    }
    else throw std::runtime_error("Too many unpins!");
}

resource_manager::resource_manager() {}

resource_manager::~resource_manager() {}

class dfo_resource_data
{
public:
    dfo_resource_data(std::unique_ptr<dfo_file>&& file)
    : file(std::move(file)) {}

    ~dfo_resource_data()
    {
        dfo_close(file.get());
    }

    void load() {}
    void unload() {}

    std::unique_ptr<dfo_file> file;
private:
};

class dfo_resource: public resource<dfo_resource_data>
{
public:
    using resource<dfo_resource_data>::resource;

    const dfo_file* get_file() const
    {
        return data().file.get();
    }
};

class dfo_buffer_reader: public buffer_data_reader
{
public:
    dfo_buffer_reader(dfo_resource res, struct dfo_buffer* buf)
    : res(res), buf(buf)
    {
    }

    size_t size() override
    {
        return buf->size;
    }

    void* loan_data() override
    {
        if(buf->present)
        {
            void* data = buf->data.data;
            buf->data.data = nullptr;
            return data;
        }
        else
        {
            char* data = new char[buf->size];
            if(!dfo_read_buffer(res.get_file(), buf, data))
            {
                delete [] data;
                throw std::runtime_error("Failed to read DFO buffer");
            }
            return data;
        }
    }

    void return_data(void* data) override
    {
        if(buf->present)
        {
            buf->data.data = data;
        }
        else
        {
            delete [] ((char*)data);
        }
    }

private:
    dfo_resource res;
    struct dfo_buffer* buf;
};

void resource_manager::add_dfo(const std::string& dfo_path)
{
    std::unique_ptr<dfo_file> fileptr(new dfo_file);
    if(!dfo_open_file(fileptr.get(), dfo_path.c_str(), 0))
    {
        throw std::runtime_error("Failed to open " + dfo_path);
    }
    dfo_resource res = create<dfo_resource>(dfo_path, std::move(fileptr));

    const dfo_file* file = res.get_file();

    // Add all buffers
    for(uint32_t i = 0; i < file->buffer_count; ++i)
    {
        struct dfo_buffer* buf = file->buffer_table[i];
    }
}

bool resource_manager::name_type::operator==(const name_type& other) const
{
    return other.type == type && other.name == name;
}

size_t resource_manager::name_type_hash::operator()(const name_type& nt) const
{
    return nt.type.hash_code() + std::hash<std::string>()(nt.name);
}
