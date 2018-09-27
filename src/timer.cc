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
#include "timer.hh"

namespace lt
{

timer::timer()
{
    start_time = clock::now();
}

duration timer::elapsed() const
{
    return clock::now() - start_time;
}

double timer::elapsed_sec() const
{
    return std::chrono::duration<double>(elapsed()).count();
}

duration timer::lap()
{
    time_point end_time = clock::now();
    duration t = end_time - start_time;
    start_time = end_time;
    return t;
}

double timer::lap_sec()
{
    return std::chrono::duration<double>(lap()).count();
}

}
