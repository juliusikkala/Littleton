#ifndef LOANER_HH
#define LOANER_HH
#include <memory>

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

#include "loaner.tcc"
#endif
