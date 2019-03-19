/*
    Copyright 2018-2019 Julius Ikkala

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
#include <stdexcept>
#include <utility>
#include <type_traits>

namespace lt
{

template<typename T, typename U, typename=void>
struct has_add_impl: std::false_type { };

template<typename T, typename U>
struct has_add_impl<
    T, U,
    decltype((void) std::declval<T>().add_impl((U*)nullptr), void())
> : std::true_type { };

template<typename T, typename U, typename=void>
struct has_remove_impl: std::false_type { };

template<typename T, typename U>
struct has_remove_impl<
    T, U,
    decltype((void) std::declval<T>().remove_impl((U*)nullptr), void())
> : std::true_type { };

template<typename T, typename=void>
struct has_clear_impl: std::false_type { };

template<typename T>
struct has_clear_impl<
    T,
    decltype((void) std::declval<T>().clear_impl(), void())
> : std::true_type { };

template<typename T, typename=void>
struct has_update_impl: std::false_type { };

template<typename T>
struct has_update_impl<
    T,
    decltype((void) std::declval<T>().update_impl(duration()), void())
> : std::true_type { };

template<typename... Scenes>
template<typename T>
void composite_scene<Scenes...>::add(T* thing)
{
    add_internal(thing, ((Scenes*)this)...);
}

template<typename... Scenes>
template<typename T, typename S, typename... Rest>
void composite_scene<Scenes...>::add_internal(T* thing, S* base, Rest*... rest)
{
    if constexpr(has_add_impl<S, T>::value)
        base->add_impl(thing);

    add_internal(thing, rest...);
}

template<typename... Scenes>
template<typename T>
void composite_scene<Scenes...>::add_internal(T* thing) {}

template<typename... Scenes>
template<typename T>
void composite_scene<Scenes...>::remove(T* thing)
{
    remove_internal(thing, ((Scenes*)this)...);
}

template<typename... Scenes>
template<typename T, typename S, typename... Rest>
void composite_scene<Scenes...>::remove_internal(
    T* thing, S* base, Rest*... rest
){
    if constexpr(has_remove_impl<S, T>::value)
        base->remove_impl(thing);

    remove_internal(thing, rest...);
}

template<typename... Scenes>
template<typename T>
void composite_scene<Scenes...>::remove_internal(T* thing) {}

template<typename... Scenes>
void composite_scene<Scenes...>::clear_all()
{
    clear_internal(((Scenes*)this)...);
}

template<typename... Scenes>
template<typename S, typename... Rest>
void composite_scene<Scenes...>::clear_internal(S* base, Rest*... rest)
{
    if constexpr(has_clear_impl<S>::value)
        base->clear_impl();
}

template<typename... Scenes>
void composite_scene<Scenes...>::clear_internal() {}

template<typename... Scenes>
void composite_scene<Scenes...>::update_all(duration delta)
{
    update_internal(delta, ((Scenes*)this)...);
}

template<typename... Scenes>
template<typename S, typename... Rest>
void composite_scene<Scenes...>::update_internal(
    duration delta, S* base, Rest*... rest
){
    if constexpr(has_update_impl<S>::value)
        base->update_impl(delta);
}

template<typename... Scenes>
void composite_scene<Scenes...>::update_internal(duration delta) {}

template<typename... Scenes>
template<typename T>
void composite_scene<Scenes...>::add_impl(T* thing) { add(thing); }
template<typename... Scenes>
template<typename T>
void composite_scene<Scenes...>::remove_impl(T* thing) { remove(thing); }
template<typename... Scenes>
void composite_scene<Scenes...>::clear_impl() { clear_all(); }
template<typename... Scenes>
void composite_scene<Scenes...>::update_impl(duration delta)
{ update_all(delta); }

}
