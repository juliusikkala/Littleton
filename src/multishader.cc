#include "multishader.hh"
#include <boost/filesystem.hpp>

multishader::multishader(
    context& ctx,
    const std::string& vert_src,
    const std::string& frag_src,
    const std::vector<std::string>& include_path
): glresource(ctx), vert_src(vert_src), frag_src(frag_src),
   include_path(include_path)
{}

multishader::multishader(multishader&& other)
: glresource(other.get_context()),
  vert_src(std::move(other.vert_src)),
  frag_src(std::move(other.frag_src)),
  include_path(std::move(other.include_path))
{}

void multishader::clear()
{
    cache.clear();
}

shader* multishader::get(const shader::definition_map& definitions) const
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

multishader* multishader::create_from_file(
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
    return new multishader(
        ctx,
        read_text_file(vert_path),
        read_text_file(frag_path),
        extended_include_path
    );
}
