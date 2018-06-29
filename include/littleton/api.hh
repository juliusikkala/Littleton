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
#ifndef LT_API_HH
#define LT_API_HH

#if defined _WIN32 || defined __CYGWIN__
#  ifdef LT_API_EXPORT
#    define LT_API __declspec(dllexport)
#    define LT_API_TEMPLATE
#  else
#    define LT_API __declspec(dllimport)
#    define LT_API_TEMPLATE extern
#  endif
#else
#  if __GNUC__ >= 4
#    define LT_API __attribute__ ((visibility ("default")))
#  else
#    define LT_API
#  endif
#  define LT_API_TEMPLATE
#endif

#endif
