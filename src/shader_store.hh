#ifndef SHADER_STORE_HH
#define SHADER_STORE_HH
#include "multishader.hh"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class shader_store
{
private:
    using map_type = std::unordered_map<
        shader::path,
        std::unique_ptr<multishader>,
        boost::hash<shader::path>
    >;

public:
    using iterator = map_type::iterator;
    using const_iterator = map_type::const_iterator;

    shader_store(
        context& ctx,
        const std::vector<std::string>& shader_path = {}
    );
    shader_store(const shader_store& other) = delete;
    shader_store(shader_store& other) = delete;
    ~shader_store();

    multishader* add(const shader::path& path);

    // Unsafe, deletes the pointer to the resource. Make sure there are no
    // references to it left.
    void remove(const shader::path& path);

    multishader* get(const shader::path& path);

    size_t size() const;

    iterator begin();
    iterator end();

    const_iterator cbegin() const;
    const_iterator cend() const;

private:
    context* ctx;
    std::vector<std::string> shader_path;
    map_type shaders;
};

#endif

