#ifndef LT_OBJECT_HH
#define LT_OBJECT_HH
#include "transformable.hh"

namespace lt
{

class model;
class object: public transformable_node
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

