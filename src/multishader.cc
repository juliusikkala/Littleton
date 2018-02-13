#include "multishader.hh"
#include <boost/filesystem.hpp>

multishader::multishader(
    context& ctx,
    const shader::source& source,
    const std::vector<std::string>& include_path
): glresource(ctx), source(source), include_path(include_path)
{}

multishader::multishader(
    context& ctx,
    const shader::path& path,
    const std::vector<std::string>& include_path
): glresource(ctx), source(path), include_path(include_path)
{
    std::vector<std::string> extended_include_path = {
    };

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
    if(it == cache.end())
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
    return it->second.get();
}
