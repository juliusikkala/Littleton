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
#ifndef LT_LOANER_HH
#define LT_LOANER_HH
#include <memory>

namespace lt
{

template<typename Resource, typename Pool>
class loan_returner
{
public:
    loan_returner();
    loan_returner(Pool& return_target);

    void operator()(Resource* res);

private:
    Pool* return_target;
};

template<typename Resource, typename Pool>
using loaner = std::unique_ptr<Resource, loan_returner<Resource, Pool>>;

} // namespace lt

#include "loaner.tcc"
#endif
