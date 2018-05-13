#include "gpu_buffer.hh"
#include <cstring>
#include <stdexcept>

namespace lt
{

gpu_buffer::gpu_buffer(context& ctx)
: glresource(ctx), buf(0), size(0)
{
}

gpu_buffer::gpu_buffer(
    context& ctx, GLenum target, size_t size, const void* data
): glresource(ctx), buf(0), size(size)
{
    basic_load(target, size, data);
}

gpu_buffer::gpu_buffer(gpu_buffer&& other)
: glresource(other.get_context())
{
    other.load();
    buf = other.buf;
    other.buf = 0;
}

gpu_buffer::~gpu_buffer()
{
    basic_unload();
}

GLuint gpu_buffer::get_buffer() const
{
    load();
    return buf;
}

GLenum gpu_buffer::get_target() const
{
    load();
    return target;
}

size_t gpu_buffer::get_size() const
{
    load();
    return size;
}

void gpu_buffer::bind() const
{
    load();
    glBindBuffer(target, buf);
}

class data_gpu_buffer: public gpu_buffer
{
public:
    data_gpu_buffer(
        context& ctx,
        GLenum target,
        size_t size,
        const void* data
    ):  gpu_buffer(ctx)
    {
        this->target = target;
        this->size = size;
        this->data = new uint8_t[size];
        memcpy(this->data, data, size);
    }

    ~data_gpu_buffer()
    {
        if(data) delete [] (uint8_t*)data;
    }

protected:
    void load_impl() const override
    {
        basic_load(target, size, data);
    }

    void unload_impl() const override
    {
        basic_unload();
    }

private:
    void* data;
};

gpu_buffer* gpu_buffer::create(
    context& ctx,
    GLenum target,
    size_t size,
    const void* data
){
    return new data_gpu_buffer(ctx, target, size, data);
}

void gpu_buffer::basic_load(
    GLenum target,
    size_t size,
    const void* data
) const
{
    if(buf) return;

    this->target = target;
    this->size = size;

    glGenBuffers(1, &buf);
    glBindBuffer(target, buf);
    glBufferData(target, size, data, GL_STATIC_DRAW);

    if(glGetError() != GL_NO_ERROR)
        throw std::runtime_error("Failed to create a gpu buffer");
}

void gpu_buffer::basic_unload() const
{
    if(buf != 0)
    {
        glDeleteBuffers(1, &buf);
        buf = 0;
    }
}

} // namespace lt
