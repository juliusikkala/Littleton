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
#ifndef LT_ANIMATED_HH
#define LT_ANIMATED_HH
#include "api.hh"
#include "timer.hh"

namespace lt
{

class LT_API animated
{
public:
    animated();

    // Set to zero for pause.
    void set_animation_multiplier(double multiplier);
    double get_animation_multiplier() const;

    void animation_update(duration delta);

    void reset_animation(duration value = {});

    duration get_animation_time() const;
    double get_animation_time_sec() const;

private:
    double multiplier;
    duration total_time;
};

} // namespace lt

#endif


