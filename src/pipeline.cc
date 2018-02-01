#include "pipeline.hh"
#include <utility>

pipeline_method::pipeline_method(render_target& target): target(&target) {}
pipeline_method::~pipeline_method() {}

void pipeline_method::set_target(render_target& target)
{
    this->target = &target;
}

render_target& pipeline_method::get_target()
{
    return *target;
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
        method->get_target().bind();
        method->execute();
        if(glGetError() != GL_NO_ERROR)
            throw std::runtime_error(
                "Error in pipeline method "
                + std::string(typeid(method).name())
            );
    }
}

