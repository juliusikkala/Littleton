#include "shader_cache.hh"
#include <boost/filesystem.hpp>

shader_cache::shader_cache(
    context& ctx,
    const std::string& vert_src,
    const std::string& frag_src,
    const std::vector<std::string>& include_path
): glresource(ctx), vert_src(vert_src), frag_src(frag_src),
   include_path(include_path)
{}

shader_cache::shader_cache(shader_cache&& other)
: glresource(other.get_context()),
  vert_src(std::move(other.vert_src)),
  frag_src(std::move(other.frag_src)),
  include_path(std::move(other.include_path))
{}

void shader_cache::clear()
{
    cache.clear();
}

shader* shader_cache::get(const shader::definition_map& definitions) const
{
    auto it = cache.find(definitions);
    if(it == cache.end())
    {
        shader* s = new shader(
            get_context(),
            vert_src,
            frag_src,
            definitions,
            include_path
        );
        cache[definitions].reset(s);
        return s;
    }
    return it->second.get();
}

shader_cache* shader_cache::create_from_file(
    context& ctx,
    const std::string& vert_path,
    const std::string& frag_path,
    const std::vector<std::string>& include_path
){
    std::vector<std::string> extended_include_path = {
        boost::filesystem::path(vert_path).parent_path().string(),
        boost::filesystem::path(frag_path).parent_path().string()
    };
    extended_include_path.insert(
        extended_include_path.end(),
        include_path.begin(),
        include_path.end()
    );
    return new shader_cache(
        ctx,
        read_text_file(vert_path),
        read_text_file(frag_path),
        extended_include_path
    );
}
