#include "multishader.hh"
#include "helpers.hh"
#include "shader.hh"
#include <boost/filesystem.hpp>

namespace lt
{

multishader::multishader(
    context& ctx,
    const shader::source& source,
    const std::vector<std::string>& include_path,
    const std::optional<std::string>& shader_binary_path
): glresource(ctx), source(source), include_path(include_path),
   shader_binary_path(shader_binary_path)
{}

multishader::multishader(
    context& ctx,
    const shader::path& path,
    const std::vector<std::string>& include_path,
    const std::optional<std::string>& shader_binary_path
): glresource(ctx), source(path), include_path(include_path),
   shader_binary_path(shader_binary_path)
{
    this->include_path.push_back(
        boost::filesystem::path(path.vert).parent_path().string()
    );
    this->include_path.push_back(
        boost::filesystem::path(path.frag).parent_path().string()
    );
    this->include_path.push_back(
        boost::filesystem::path(path.geom).parent_path().string()
    );
}

multishader::multishader(multishader&& other)
: glresource(other.get_context()),
  source(std::move(other.source)),
  include_path(std::move(other.include_path)),
  cache(std::move(other.cache))
{}

void multishader::clear()
{
    cache.clear();
}

void multishader::unload()
{
    for(auto& pair: cache) pair.second->unload();
}

shader* multishader::get(const shader::definition_map& definitions) const
{
    auto it = cache.find(definitions);
    // Cache miss
    if(it == cache.end())
    {
        if(shader_binary_path)
        {
            boost::filesystem::path path(append_hash_to_path(
                shader_binary_path.value(),
                definitions,
                ".bin"
            ));

            shader* s = shader::create(
                get_context(),
                source,
                definitions,
                include_path,
                path.string()
            );

            cache[definitions].reset(s);

            return s;
        }
        else
        {
            shader* s = shader::create(
                get_context(),
                source,
                definitions,
                include_path
            );
            cache[definitions].reset(s);
            return s;
        }
    }
    return it->second.get();
}

} // namespace lt
