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
