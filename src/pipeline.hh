#ifndef PIPELINE_HH
#define PIPELINE_HH
#include <vector>
#include <memory>
#include "render_target.hh"

class pipeline_method
{
public:
    pipeline_method(render_target& target);
    virtual ~pipeline_method();

    void set_target(render_target& target);
    render_target& get_target();

    virtual void execute() = 0;
private:
    render_target* target;
};

class pipeline
{
public:
    pipeline(const std::vector<pipeline_method*>& methods);
    pipeline(pipeline&& other);
    ~pipeline();

    void execute();

private:
    std::vector<pipeline_method*> methods;
};

#endif
