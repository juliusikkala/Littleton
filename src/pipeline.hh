#ifndef PIPELINE_HH
#define PIPELINE_HH
#include <vector>
#include <memory>
#include "render_target.hh"

class pipeline_method
{
public:
    virtual ~pipeline_method();

    virtual void execute() = 0;
};

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

private:
    std::vector<pipeline_method*> methods;
};

#endif
