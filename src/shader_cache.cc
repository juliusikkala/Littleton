#include "shader_cache.hh"

shader_cache::shader_cache(
    context& ctx,
    const std::string& vert_src,
    const std::string& frag_src
): glresource(ctx), vert_src(vert_src), frag_src(frag_src)
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
        shader* s = new shader(get_context(), vert_src, frag_src, definitions);
        cache[definitions].reset(s);
        return s;
    }
    return it->second.get();
}
