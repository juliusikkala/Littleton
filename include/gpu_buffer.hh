#ifndef LT_GPU_BUFFER_HH
#define LT_GPU_BUFFER_HH
#include "glheaders.hh"
#include "resource.hh"

namespace lt
{

class gpu_buffer: public resource, public glresource
{
public:
    explicit gpu_buffer(context& ctx);
    gpu_buffer(context& ctx, GLenum target, size_t size, const void* data);
    gpu_buffer(gpu_buffer&& other);
    ~gpu_buffer();

    GLuint get_buffer() const;
    GLenum get_target() const;
    size_t get_size() const;

    void bind() const;

    // Creates a lazily loaded buffer. Takes ownership of the pointers.
    static gpu_buffer* create(
        context& ctx,
        GLenum target,
        size_t size,
        const void* data
    );

protected:
    void basic_load(
        GLenum target,
        size_t size,
        const void* data
    ) const;

    void basic_unload() const;

    mutable GLuint buf;
    mutable GLenum target;
    mutable size_t size;
};

} // namespace lt

#endif

