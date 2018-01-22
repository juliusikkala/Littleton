#ifndef PIPELINE_HH
#define PIPELINE_HH
#include <vector>
#include <memory>

class pipeline_method
{
public:
    virtual ~pipeline_method();
    virtual void execute() = 0;
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
