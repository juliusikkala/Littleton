#ifndef MULTISHADER_HH
#define MULTISHADER_HH
#include "shader.hh"
#include "helpers.hh"
#include <map>
#include <unordered_map>
#include <memory>
#include <boost/functional/hash.hpp>

class multishader: public glresource
{
public:
    multishader(
        context& ctx,
        const shader::source& source,
        const std::vector<std::string>& include_path = {}
    );
    multishader(
        context& ctx,
        const shader::path& path,
        const std::vector<std::string>& include_path = {}
    );
    multishader(multishader&& other);

    void clear();
    shader* get(const shader::definition_map& definitions = {}) const;

private:
    shader::source source;
    std::vector<std::string> include_path;
    mutable std::unordered_map<
        shader::definition_map,
        std::unique_ptr<shader>,
        boost::hash<shader::definition_map>
    > cache;
};

#endif
