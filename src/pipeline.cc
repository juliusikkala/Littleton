/*
    Copyright 2018 Julius Ikkala

    This file is part of Littleton.

    Littleton is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Littleton is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Littleton.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "pipeline.hh"
#include "render_target.hh"
#include <utility>
#include <stdexcept>
#include <typeinfo>
#include <string>
#include <boost/core/demangle.hpp>

namespace lt
{

pipeline_method::~pipeline_method() {}
std::string pipeline_method::get_name() const
{
    return boost::core::demangle(typeid(*this).name());
}

target_method::target_method(render_target& target): target(&target) {}

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
                + method->get_name() + " index " + std::to_string(i)
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
                + method->get_name() + " index " + std::to_string(i)
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

std::string pipeline::get_name(size_t i) const
{
    return methods[i]->get_name();
}

std::string pipeline::get_name() const
{
    return "pipeline";
}

} // namespace lt
