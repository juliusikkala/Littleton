#include "resources.hh"
#include <stdexcept>
#include <memory>
#include "dfo.h"
#include "buffer.hh"
#include "texture.hh"
#include <map>

basic_resource_ptr::~basic_resource_ptr()
{
    reset(nullptr);
}

void basic_resource_ptr::pin() const
{
    if(s)
    {
        local_pins++;
        s->global_pins++;
        if(s->global_pins == 1)
        {
            s->resource = s->create_resource();
        }
    }
}

void basic_resource_ptr::unpin() const
{
    if(s && local_pins != 0)
    {
        local_pins--;
        s->global_pins--;
        if(s->global_pins == 0)
        {
            s->delete_resource(s->resource);
            s->resource = nullptr;
        }
    }
}

bool basic_resource_ptr::operator==(const basic_resource_ptr& other) const
{
    return s && other.s == s;
}

bool basic_resource_ptr::operator!=(const basic_resource_ptr& other) const
{
    return !s || other.s != s;
}

basic_resource_ptr::operator bool() const
{
    return s;
}

basic_resource_ptr::basic_resource_ptr()
: local_pins(0), s(nullptr) { }

basic_resource_ptr::basic_resource_ptr(shared* other_s)
: local_pins(0), s(other_s) { s->references++; }

basic_resource_ptr::basic_resource_ptr(const basic_resource_ptr& other)
: local_pins(0), s(other.s) { s->references++; }

basic_resource_ptr::basic_resource_ptr(basic_resource_ptr&& other)
: local_pins(other.local_pins), s(other.s)
{
    other.local_pins = 0;
    other.s = nullptr;
}

void basic_resource_ptr::reset(shared* other_s)
{
    if(s)
    {
        s->global_pins -= local_pins;
        if(s->global_pins == 0 && s->resource)
        {
            s->delete_resource(s->resource);
            s->resource = nullptr;
        }

        s->references--;
        if(s->references == 0)
        {
            if(s->resource)
            {
                s->delete_resource(s->resource);
            }
            delete s;
        }
    }

    local_pins = 0;
    s = other_s;
    if(s) s->references++;
}

basic_resource_ptr& basic_resource_ptr::operator=(basic_resource_ptr&& other)
{
    if(s)
    {
        s->global_pins -= local_pins;
        if(s->global_pins == 0 && s->resource)
        {
            s->delete_resource(s->resource);
            s->resource = nullptr;
        }

        s->references--;
        if(s->references == 0)
        {
            if(s->resource)
            {
                s->delete_resource(s->resource);
            }
            delete s;
        }
    }
    s = other.s;
    local_pins = other.local_pins;

    other.local_pins = 0;
    other.s = nullptr;
    return *this;
}

basic_resource_ptr& basic_resource_ptr::operator=(
    const basic_resource_ptr& other
){
    reset(other.s);
    return *this;
}


resource_store::resource_store() { }

resource_store::~resource_store() { }

class dfo_file_resource
{
public:
    dfo_file_resource(const std::string& path)
    {
        if(!dfo_open_file(&file, path.c_str(), 0))
        {
            throw std::runtime_error("Failed to open " + path);
        }
    }

    ~dfo_file_resource()
    {
        dfo_close(&file);
    }

    const dfo_file* get_file() const
    {
        return &file;
    }
private:
    dfo_file file;
};

using dfo_ptr = resource_ptr<dfo_file_resource>;

class dfo_buffer_reader
{
public:
    dfo_buffer_reader(dfo_ptr res, dfo_buffer* buf)
    : res(res), buf(buf)
    {
        this->res.pin();
    }

    dfo_buffer_reader(const dfo_buffer_reader& other)
    : res(other.res), buf(other.buf)
    {
        this->res.pin();
    }

    ~dfo_buffer_reader()
    {
    }

    void* operator()()
    {
        buffer* new_buf;
        if(buf->present)
        {
            new_buf = new buffer(
                (buffer::buffer_type)buf->type,
                buf->data.data,
                buf->size
            );
        }
        else
        {
            char* data = new char[buf->size];
            if(!dfo_read_buffer(res->get_file(), buf, data))
            {
                delete [] data;
                throw std::runtime_error("Failed to read DFO buffer");
            }
            new_buf = new buffer(
                (buffer::buffer_type)buf->type,
                data,
                buf->size
            );
            delete [] data;
        }
        return new_buf;
    }

private:
    dfo_ptr res;
    dfo_buffer* buf;
};

void resource_store::add_dfo(const std::string& dfo_path)
{
    dfo_ptr res = create<dfo_file_resource>(dfo_path, dfo_path);

    const dfo_file* file = res->get_file();

    // Add all buffers
    std::map<dfo_buffer*, buffer_ptr> buffers;

    for(uint32_t i = 0; i < file->buffer_count; ++i)
    {
        dfo_buffer* buf = file->buffer_table[i];
        buffers[buf] = buffer_ptr(dfo_buffer_reader(res, buf));
    }

    // Add all textures
    std::map<dfo_texture*, texture_ptr> textures;

    for(uint32_t i = 0; i < file->texture_count; ++i)
    {
        dfo_texture* tex = file->texture_table[i];
        textures[tex] = create<texture>(tex->path, std::string(tex->path));
    }
}

void resource_store::pin_all()
{
    for(auto& pair: resources)
    {
        pair.second.pin();
    }
}

void resource_store::unpin_all()
{
    for(auto& pair: resources)
    {
        pair.second.unpin();
    }
}

bool resource_store::name_type::operator==(const name_type& other) const
{
    return other.type == type && other.name == name;
}

size_t resource_store::name_type_hash::operator()(const name_type& nt) const
{
    return nt.type.hash_code() + std::hash<std::string>()(nt.name);
}

