#ifndef LT_RESOURCE_HH
#define LT_RESOURCE_HH

namespace lt
{

// Only useful for lazily loadable resources.
// Since OpenGL should only be used from the main thread, this class doesn't
// concern itself with thread safety.
// Deriving classes should call unload_impl() themselves at the end of their
// destructors, if necessary.
class resource
{
public:
    resource();
    virtual ~resource();

    void load() const;
    // This will not actually unload the resource if it is still required by
    // some other resource. If that is the case, the resource will be unloaded
    // once it is no longer needed by other resources.
    void unload() const;

    // Forcibly unloads the resource, may be unsafe. Should only be used when
    // __all__ other possibly linked resources are also force_unload():ed, such
    // as when closing the program or deleting a level's resources.
    void force_unload() const;

    bool is_loaded() const;

    // Increments reference count. This does not automatically load the
    // resource, it can be done manually if needed. Typically resources load
    // themselves once you use them.
    void link() const;

    // Decrements reference count. If the count hits zero, unloads the object if
    // it is marked for unloading.
    void unlink() const;

protected:
    virtual void load_impl() const;
    virtual void unload_impl() const;

private:
    mutable bool loaded;
    mutable bool do_unload;
    mutable unsigned references;
};

class context;
class glresource
{
public:
    explicit glresource(context& ctx);

    context& get_context() const;
private:
    context* ctx;
};

} // namespace lt

#endif
