#include "loaner.hh"

namespace lt
{

template<typename Resource, typename Pool>
loan_returner<Resource, Pool>::loan_returner()
: return_target(nullptr) { }

template<typename Resource, typename Pool>
loan_returner<Resource, Pool>::loan_returner(Pool& return_target)
: return_target(&return_target) { }

template<typename Resource, typename Pool>
void loan_returner<Resource, Pool>::operator()(Resource* res)
{
    if(return_target) return_target->give(res);
}

} // namespace lt
