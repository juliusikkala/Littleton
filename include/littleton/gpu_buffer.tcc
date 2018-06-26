#include <cstring>

namespace lt
{

template<typename T>
std::vector<T> gpu_buffer::read() const
{
    bind();
    void* data = glMapBuffer(target, GL_READ_ONLY);
    std::vector<T> res(size/sizeof(T));
    memcpy(res.data(), data, size);
    glUnmapBuffer(target);
    return res;
}

} // namespace lt
