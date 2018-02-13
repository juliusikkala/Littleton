#include "pipeline.hh"
#include <utility>

pipeline_method::~pipeline_method() {}

target_method::target_method(render_target& target): target(&target) {}

void target_method::set_target(render_target& target)
{
    this->target = &target;
}

render_target& target_method::get_target()
{
    return *target;
}

void target_method::execute()
{
    target->bind();
}

pipeline::pipeline(const std::vector<pipeline_method*>& methods)
: methods(methods) {}

pipeline::pipeline(pipeline&& other)
: methods(std::move(other.methods))
{}

pipeline::~pipeline() {}

void pipeline::execute()
{
    for(auto& method: methods)
    {
        method->execute();
        if(glGetError() != GL_NO_ERROR)
            throw std::runtime_error(
                "Error in pipeline method "
                + std::string(typeid(method).name())
            );
    }
}

