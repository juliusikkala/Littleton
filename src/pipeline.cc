#include "pipeline.hh"
#include "render_target.hh"
#include <utility>
#include <stdexcept>
#include <typeinfo>
#include <string>

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
    for(unsigned i = 0; i < methods.size(); ++i)
    {
        pipeline_method* method = methods[i];
        method->execute();
        if(glGetError() != GL_NO_ERROR)
            throw std::runtime_error(
                "Error in pipeline method "
                + std::string(typeid(method).name())
                + " index " + std::to_string(i)
            );
    }
}

void pipeline::execute(std::vector<double>& timing)
{
    std::vector<GLuint> queries(methods.size());
    glGenQueries(methods.size(), queries.data());
    timing.resize(methods.size());

    for(unsigned i = 0; i < methods.size(); ++i)
    {
        pipeline_method* method = methods[i];

        glBeginQuery(GL_TIME_ELAPSED, queries[i]);
        method->execute();
        glEndQuery(GL_TIME_ELAPSED);

        if(glGetError() != GL_NO_ERROR)
            throw std::runtime_error(
                "Error in pipeline method "
                + std::string(typeid(method).name())
            );
    }

    for(unsigned i = 0; i < methods.size(); ++i)
    {
        int time = 0;
        glGetQueryObjectiv(queries[i], GL_QUERY_RESULT, &time);

        timing[i] = time / 1000000.0f;
    }
    glDeleteQueries(methods.size(), queries.data());
}
