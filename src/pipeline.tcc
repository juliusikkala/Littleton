#include <utility>

template<typename... Args>
pipeline::pipeline(Args&&... rest)
{
    append_method(std::forward<Args>(rest)...);
}

template<typename T, typename... Args>
void pipeline::append_method(T&& method, Args&&... rest)
{
    methods.emplace_back(method.pipeline_build(*this));
    append_method(std::forward<Args>(rest)...);
}
