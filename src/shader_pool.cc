#include "shader_pool.hh"
#include "multishader.hh"
#include "helpers.hh"
#include <boost/filesystem.hpp>

namespace lt
{

static std::string find_file(
    const std::vector<std::string> prefixes,
    const std::string& suffix
){
    boost::filesystem::path suffix_path(suffix);
    for(std::string prefix: prefixes)
    {
        boost::filesystem::path prefix_path(prefix);
        boost::filesystem::path combined(prefix_path/suffix_path);
        
        if(boost::filesystem::exists(combined))
            return combined.string();
    }
    throw std::runtime_error("Unable to find shader source " + suffix);
}

shader_pool::shader_pool(
    context& ctx,
    const std::vector<std::string>& shader_path,
    const std::optional<std::string>& shader_binary_path
):  glresource(ctx), shader_path(shader_path),
    shader_binary_path(shader_binary_path)
{}

shader_pool::~shader_pool()
{
}

multishader* shader_pool::add(const shader::path& path)
{
    auto it = shaders.find(path);
    if(it != shaders.end())
        throw std::runtime_error(
            "Shader with source files [" + path.vert + ", " + path.frag +
            "] already exists!"
        );

    shader::path full_path{
        find_file(shader_path, path.vert),
        find_file(shader_path, path.frag),
        path.geom.empty() ? "" : find_file(shader_path, path.geom)
    };

    multishader* s;
    if(shader_binary_path)
    {
        s = new multishader(
            get_context(),
            full_path,
            shader_path,
            append_hash_to_path(shader_binary_path.value(), full_path)
        ); 
    }
    else s = new multishader(get_context(), full_path, shader_path); 

    shaders[path] = std::unique_ptr<multishader>(s);
    return s;
}

void shader_pool::remove(const shader::path& path)
{
    shaders.erase(path);
}

void shader_pool::delete_binaries()
{
    if(shader_binary_path)
    {
        boost::filesystem::remove_all(shader_binary_path.value());
    }
}

void shader_pool::unload_all()
{
    for(auto& pair: shaders)
    {
        pair.second->unload();
    }
}

multishader* shader_pool::get(const shader::path& path)
{
    auto it = shaders.find(path);
    if(it == shaders.end())
        return add(path);
    return it->second.get();
}

shader* shader_pool::get(
    const shader::path& path,
    const shader::definition_map& definitions
){
    return get(path)->get(definitions);
}

shader_pool::iterator shader_pool::begin() { return shaders.begin(); }
shader_pool::iterator shader_pool::end() { return shaders.end(); }

shader_pool::const_iterator shader_pool::cbegin() const
{
    return shaders.cbegin(); 
}

shader_pool::const_iterator shader_pool::cend() const
{
    return shaders.cend();
}

} // namespace lt
