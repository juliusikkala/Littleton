#ifndef SHADER_CACHE_HH
#define SHADER_CACHE_HH
#include "shader.hh"
#include "helpers.hh"
#include <map>
#include <unordered_map>
#include <memory>
#include <boost/functional/hash.hpp>

class shader_cache: public glresource
{
public:
    shader_cache(
        context& ctx,
        const std::string& vert_src,
        const std::string& frag_src,
        const std::vector<std::string>& include_path = {}
    );
    shader_cache(shader_cache&& other);

    static shader_cache* create_from_file(
        context& ctx,
        const std::string& vert_path,
        const std::string& frag_path,
        const std::vector<std::string>& include_path = {}
    );

    void clear();
    shader* get(const shader::definition_map& definitions = {}) const;

private:
    std::string vert_src, frag_src;
    std::vector<std::string> include_path;
    mutable std::unordered_map<
        shader::definition_map,
        std::unique_ptr<shader>,
        boost::hash<shader::definition_map>
    > cache;
};

#endif
