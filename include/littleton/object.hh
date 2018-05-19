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
#ifndef LT_OBJECT_HH
#define LT_OBJECT_HH
#include "../api.hh"
#include "transformable.hh"

namespace lt
{

class model;
class LT_API object: public transformable_node
{
public:
    object(const model* mod = nullptr, transformable_node* parent = nullptr);
    ~object();

    /* Be extra careful when using this function. Make sure that 'model'
     * outlives this object or is unset before its destruction.
     */
    void set_model(const model* mod = nullptr);
    const model* get_model() const;

private:
    const model* mod;
};

} // namespace lt

#endif

