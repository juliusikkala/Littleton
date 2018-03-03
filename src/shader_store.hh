#ifndef SHADER_STORE_HH
#define SHADER_STORE_HH
#include "shader.hh"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <boost/functional/hash.hpp>

class multishader;
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
    shader_store(
        context& ctx,
        const std::vector<std::string>& shader_path,
        const std::string& shader_binary_path
    );
    shader_store(const shader_store& other) = delete;
    shader_store(shader_store& other) = delete;
    ~shader_store();

    multishader* add(const shader::path& path);

    // Unsafe, deletes the pointer to the resource. Make sure there are no
    // references to it left. You are probably looking for shader::unload().
    void remove(const shader::path& path);

    // This deletes all binaries in shader_binary_path
    void delete_binaries();

    // Unloads all shaders in this store.
    void unload_all();

    multishader* get(const shader::path& path);
    shader* get(
        const shader::path& path,
        const shader::definition_map& definitions
    );

    size_t size() const;

    iterator begin();
    iterator end();

    const_iterator cbegin() const;
    const_iterator cend() const;

private:
    context* ctx;
    std::vector<std::string> shader_path;
    std::optional<std::string> shader_binary_path;
    map_type shaders;
};

#endif

