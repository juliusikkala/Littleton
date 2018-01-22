#include "pipeline.hh"
#include <utility>

pipeline_method::~pipeline_method() {}

pipeline::pipeline(const std::vector<pipeline_method*>& methods)
: methods(methods) {}

pipeline::pipeline(pipeline&& other)
: methods(std::move(other.methods))
{}

pipeline::~pipeline() {}

void pipeline::execute()
{
    for(auto& method: methods) method->execute();
}

