#ifndef RESOURCES_HH
#define RESOURCES_HH
#include <unordered_map>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <functional>

class basic_resource_ptr
{
friend class resource_store;
public:
    basic_resource_ptr();
    ~basic_resource_ptr();

    void pin() const;
    void unpin() const;


    bool operator<(const basic_resource_ptr& other) const;
    bool operator<=(const basic_resource_ptr& other) const;
    bool operator>(const basic_resource_ptr& other) const;
    bool operator>=(const basic_resource_ptr& other) const;
    bool operator==(const basic_resource_ptr& other) const;
    bool operator!=(const basic_resource_ptr& other) const;

    operator bool() const;

protected:
    struct shared
    {
        unsigned references;
        unsigned global_pins;
        std::function<void*()> create_resource;
        std::function<void(void*)> delete_resource;
        void* resource;
    };

    basic_resource_ptr(shared* other_s);
    basic_resource_ptr(const basic_resource_ptr& other);
    basic_resource_ptr(basic_resource_ptr&& other);

    void reset(shared* other_s);

    basic_resource_ptr& operator=(basic_resource_ptr&& other);
    basic_resource_ptr& operator=(const basic_resource_ptr& other);

    mutable unsigned local_pins;
    mutable shared* s;
};

template<typename T>
class resource_ptr: public basic_resource_ptr
{
friend class resource_store;
public:
    resource_ptr();

    template<
        typename U,
        typename = std::enable_if_t<std::is_convertible_v<U*, T*>>
    >
    resource_ptr(const resource_ptr<U>& other);

    template<
        typename U,
        typename = std::enable_if_t<std::is_convertible_v<U*, T*>>
    >
    resource_ptr(resource_ptr<U>&& other);

    explicit resource_ptr(
        std::function<void*()> create_resource,
        std::function<void(void*)> delete_resource =
            [](void* res){ delete ((T*)res);}
    );

    /* Takes ownership of the pointer. Note that lazy loading will not
     * work in this case, the data will be released only when the last
     * resource_ptr referring to it is being destructed.
     */
    explicit resource_ptr(T* ptr);

    /* Doesn't take ownership! Make sure the reference outlives this
     * resource_ptr and all its copies!
     */
    explicit resource_ptr(T& reference);

    /* Make sure that the arguments can be safely copied and will be usable
     * during the lifetime of the resource. Be extra careful with pointers,
     * including C-style strings. String literals should be safe though,
     * they're stored statically according to standard.
     */
    template<typename... Args>
    static resource_ptr<T> create(Args&&... args);

    T& operator*() const;

    T* operator->() const;

    T* get() const;

    resource_ptr<T>& operator=(T* other);
    resource_ptr<T>& operator=(T& other);
    template<
        typename U,
        typename = std::enable_if_t<std::is_convertible_v<U*, T*>>
    >
    resource_ptr<T>& operator=(resource_ptr<U>&& other);

    template<
        typename U,
        typename = std::enable_if_t<std::is_convertible_v<U*, T*>>
    >
    resource_ptr<T>& operator=(const resource_ptr<U>& other);

private:
    // Note that this is not type safe!
    resource_ptr(const basic_resource_ptr& other);
    resource_ptr(shared* s);
};

class resource_store
{
public:
    resource_store();
    resource_store(const resource_store& other) = delete;
    resource_store(resource_store& other) = delete;
    ~resource_store();

    template<typename T, typename... Args>
    const resource_ptr<T>& create(const std::string& name, Args&&... args);

    template<typename T>
    const resource_ptr<T>& add(const std::string& name, resource_ptr<T> res);

    /* Note that while the resource is removed from this store, it may have
     * references elsewhere and not be freed because of that.
     */
    template<typename T>
    void remove(const std::string& name);

    template<typename T>
    const resource_ptr<T>& get(const std::string& name) const;

    void add_dfo(const std::string& dfo_path);

    // Essentially makes sure that all resources in this store are loaded.
    void pin_all();
    void unpin_all();

    // These pins will stay, unlike if you pinned the resource_ptr yourself
    template<typename T>
    void pin(const std::string& name);

    template<typename T>
    void unpin(const std::string& name);

private:

    struct name_type
    {
        std::type_index type;
        std::string name;

        bool operator==(const name_type& other) const;
    };

    class name_type_hash
    {
    public:
        size_t operator()(const name_type& nt) const;
    };

    std::unordered_map<
        name_type,
        basic_resource_ptr,
        name_type_hash
    > resources;
};

#include "resources.tcc"
#endif

