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
#ifndef LT_METHOD_CLEAR_HH
#define LT_METHOD_CLEAR_HH
#include "../api.hh"
#include "../pipeline.hh"
#include "../math.hh"

namespace lt::method
{

LT_OPTIONS(clear)
{
    glm::vec4 color = glm::vec4(0);
    double depth = 1;
    int stencil = 0;
};

class LT_API clear: public target_method, public options_method<clear>
{
public:
    clear(
        render_target& target,
        const options& opt = {}
    );
    ~clear();

    void execute() override;
};

} // namespace lt::method

#endif
