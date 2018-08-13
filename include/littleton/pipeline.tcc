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

namespace lt
{

template<typename Derived>
options_method<Derived>::options_method(const options& opt)
{
    this->opt = opt;
}

template<typename Derived>
template<
    typename Self,
    typename options_method<Derived>::template member_detector<
        decltype(&Self::options_will_update)
    >::type = 0
>
void options_method<Derived>::accessor::will_update(
    Derived& method,
    const options& opt,
    member_detector_special
){
    // HACK! Takes the function pointer to the member function from
    // the parent class and calls it on the argument object.
    void (Derived::*fn)(const options& opt) = &accessor::options_will_update;
    (method.*fn)(opt);
}

template<typename Derived>
template<typename Self>
void options_method<Derived>::accessor::will_update(
    Derived&,
    const options&,
    member_detector_general
) {}

template<typename Derived>
void options_method<Derived>::set_options(const options& opt)
{
    accessor::template will_update<accessor>(
        static_cast<Derived&>(*this),
        opt,
        member_detector_special()
    );
    this->opt = opt;
}

template<typename Derived>
const typename options_method<Derived>::options&
options_method<Derived>::get_options() const
{
    return opt;
}

} // namespace lt
