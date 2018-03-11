#ifndef RESOURCE_HH
#define RESOURCE_HH

// Only useful for lazily loadable resources.
class resource
{
public:
    resource();
    virtual ~resource();

    void load() const;
    void unload() const;

    bool is_loaded() const;

protected:
    virtual void load_impl() const;
    virtual void unload_impl() const;

private:
    mutable bool loaded;
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
