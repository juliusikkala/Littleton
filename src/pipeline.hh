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
    template<typename... Args>
    pipeline(Args&&... rest);
    pipeline(pipeline&& other);
    ~pipeline();

    void execute();

private:
    void append_method();
    
    template<typename T, typename... Args>
    void append_method(T& method, Args&&... rest);

    std::vector<std::unique_ptr<pipeline_method>> methods;
};

#include "pipeline.tcc"
#endif
