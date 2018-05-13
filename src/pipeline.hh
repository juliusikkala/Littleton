#ifndef LT_PIPELINE_HH
#define LT_PIPELINE_HH
#include <vector>
#include <string>

namespace lt
{

class pipeline_method
{
public:
    virtual ~pipeline_method();

    virtual void execute() = 0;
    virtual std::string get_name() const;
};

class render_target;
class target_method: public pipeline_method
{
public:
    target_method(render_target& target);

    void set_target(render_target& target);
    render_target& get_target();

    // Call this from the deriving method with target_method::execute().
    // You can also bind the target yourself if you want to.
    virtual void execute();
private:
    render_target* target;
};

class pipeline: public pipeline_method
{
public:
    pipeline(const std::vector<pipeline_method*>& methods);
    pipeline(pipeline&& other);
    ~pipeline();


    void execute();
    void execute(std::vector<double>& timing);

    std::string get_name(size_t i) const;
    virtual std::string get_name() const;
private:
    std::vector<pipeline_method*> methods;
};

} // namespace lt

#endif
