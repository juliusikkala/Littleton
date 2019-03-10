/*
    Copyright 2019 Julius Ikkala

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
#include "animated.hh"
#include "math.hh"

namespace lt
{

animated::animated()
: multiplier(1.0f), total_time(duration::zero())
{}

void animated::set_animation_multiplier(double multiplier)
{
    this->multiplier = fabs(multiplier);
}

double animated::get_animation_multiplier() const
{
    return multiplier;
}

void animated::animation_update(duration delta)
{
    duration modified_delta =
        std::chrono::duration_cast<duration>(delta * multiplier);
    total_time += modified_delta;
}

void animated::reset_animation(duration value)
{
    total_time = value;
}

duration animated::get_animation_time() const
{
    return total_time;
}

double animated::get_animation_time_sec() const
{
    return std::chrono::duration<double>(total_time).count();
}

} // namespace lt
