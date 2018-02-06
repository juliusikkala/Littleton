#ifndef RESOURCES_HH
#define RESOURCES_HH
#include <unordered_map>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <iterator>
#include <memory>

// Not every resource in resource_store has to derive from this; only if they
// provide (optional) lazy loading is this resource class required.
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

// TODO: Break this thing up, this 'everything-container' is kinda smelly
// Maybe type-specific stores like object_store, shader_store and so on,
// add_dfo would become a separate function taking the necessary stores as
// dependencies. resource_store could be a 'master' store, either through
// deriving every other store type or containing them.
class resource_store
{
private:
    struct container
    {
        virtual ~container() = 0;
        void* data;
    };

    template<typename T>
    class typed_container: public container
    {
    public:
        typed_container(T* data);
        ~typed_container();
    };


    using inner_map = std::unordered_map<
        std::string,
        std::unique_ptr<container>
    >;
    using internal_map = std::unordered_map<std::type_index, inner_map>;
    mutable internal_map resources;
    context* ctx;

public:

    // There is nothing in this language that I hate more than iterators.
    template<typename T, typename I>
    class resource_iterator: public std::iterator<std::forward_iterator_tag, T>
    {
    public:
        resource_iterator() = default;
        resource_iterator(const resource_iterator<T, I>& other) = default;

        void swap(resource_iterator<T, I>& other) noexcept;

        resource_iterator<T, I>& operator++();
        resource_iterator<T, I> operator++(int);

        template<typename U>
        bool operator==(const resource_iterator<U, I>& other) const;

        template<typename U>
        bool operator!=(const resource_iterator<U, I>& other) const;

        T* operator*() const;

        const std::string& name() const;

        template<typename J>
        operator resource_iterator<const T, J>() const;

    private:
        template<typename U>
        friend class iterable;

        template<typename U>
        friend class const_iterable;

        friend class resource_store;

        explicit resource_iterator(I it);
        I it;
    };

    template<typename T>
    using iterator = resource_iterator<T, inner_map::iterator>;

    template<typename T>
    using const_iterator =
        resource_iterator<const T, inner_map::const_iterator>;

    resource_store(context& ctx);
    resource_store(const resource_store& other) = delete;
    resource_store(resource_store& other) = delete;
    ~resource_store();

    // Takes ownership of the pointer.
    template<typename T>
    T* add(const std::string& name, T* res);

    template<typename T>
    T* add(const std::string& name, T&& res);

    void add_dfo(
        const std::string& path,
        const std::string& data_prefix = ""
    );

    // Unsafe, deletes the pointer to the resource.
    template<typename T>
    void remove(const std::string& name);

    template<typename T>
    T* get(const std::string& name) const;

    template<typename T>
    size_t size() const;

    template<typename T>
    iterator<T> begin();

    template<typename T>
    iterator<T> end();

    template<typename T>
    const_iterator<T> cbegin() const;

    template<typename T>
    const_iterator<T> cend() const;

    template<typename T>
    class iterable
    {
    public:
        iterator<T> begin();
        iterator<T> end();
    private:
        friend class resource_store;
        iterable(inner_map& m);

        inner_map& m;
    };

    template<typename T>
    class const_iterable
    {
    public:
        const_iterator<T> cbegin() const;
        const_iterator<T> cend() const;
    private:
        friend class resource_store;
        const_iterable(const inner_map& m);

        const inner_map& m;
    };

    template<typename T>
    iterable<T> get_iterable();

    template<typename T>
    const_iterable<T> get_const_iterable() const;
};

#include "resources.tcc"
#endif
