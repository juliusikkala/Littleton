#include "pipeline.hh"
#include <utility>

pipeline_method::~pipeline_method() {}

pipeline::pipeline(pipeline&& other)
: methods(std::move(other.methods))
{}

pipeline::~pipeline() {}

void pipeline::execute()
{
    for(auto& method: methods) method->execute();
}

void pipeline::append_method() {}
