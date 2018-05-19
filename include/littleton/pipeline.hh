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
#ifndef LT_PIPELINE_HH
#define LT_PIPELINE_HH
#include "../api.hh"
#include <vector>
#include <string>

namespace lt
{

class LT_API pipeline_method
{
public:
    virtual ~pipeline_method();

    virtual void execute() = 0;
    virtual std::string get_name() const;
};

class render_target;
class LT_API target_method: public pipeline_method
{
public:
    target_method(render_target& target);

    void set_target(render_target& target);
    render_target& get_target();

    // Call this from the deriving method with target_method::execute().
    // You can also bind the target yourself if you want to.
    virtual void execute();
private:
    render_target* target;
};

class LT_API pipeline: public pipeline_method
{
public:
    pipeline(const std::vector<pipeline_method*>& methods);
    pipeline(pipeline&& other);
    ~pipeline();


    void execute();
    void execute(std::vector<double>& timing);

    std::string get_name(size_t i) const;
    virtual std::string get_name() const;
private:
    std::vector<pipeline_method*> methods;
};

} // namespace lt

#endif
