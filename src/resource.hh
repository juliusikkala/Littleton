#ifndef RESOURCE_HH
#define RESOURCE_HH

// Only useful for lazily loadable resources.
class resource
{
public:
    virtual ~resource();

    virtual void load() const;
    virtual void unload() const;
};

class context;
class glresource
{
public:
    glresource(context& ctx);

    context& get_context() const;
private:
    context* ctx;
};

#endif
