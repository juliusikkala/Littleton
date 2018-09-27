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
#ifndef LT_TIMER_HH
#define LT_TIMER_HH
#include "api.hh"
#include <chrono>

namespace lt
{

using clock = std::chrono::high_resolution_clock;
using time_point = clock::time_point;
using duration = clock::duration;

class LT_API timer
{
public:
    timer();

    duration elapsed() const;
    double elapsed_sec() const;
    duration lap();
    double lap_sec();
private:
    time_point start_time;
};

};

#endif
