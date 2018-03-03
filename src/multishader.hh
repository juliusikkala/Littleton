#ifndef MULTISHADER_HH
#define MULTISHADER_HH
#include "shader.hh"
#include "helpers.hh"
#include <map>
#include <unordered_map>
#include <memory>
#include <optional>
#include <boost/functional/hash.hpp>

class multishader: public glresource
{
public:
    multishader(
        context& ctx,
        const shader::source& source,
        const std::vector<std::string>& include_path = {},
        const std::optional<std::string>& shader_binary_path = {}
    );

    multishader(
        context& ctx,
        const shader::path& path,
        const std::vector<std::string>& include_path = {},
        const std::optional<std::string>& shader_binary_path = {}
    );

    multishader(multishader&& other);

    // This is slightly dangerous if someone is keeping a reference to a
    // shader. Please use unload instead.
    void clear();

    // Unloads all shaders in the cache.
    void unload();

    shader* get(const shader::definition_map& definitions = {}) const;

private:
    shader::source source;

    std::vector<std::string> include_path;
    std::optional<std::string> shader_binary_path;

    mutable std::unordered_map<
        shader::definition_map,
        std::unique_ptr<shader>,
        boost::hash<shader::definition_map>
    > cache;
};

#endif
