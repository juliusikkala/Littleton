#include "shader_store.hh"
#include <boost/filesystem.hpp>

    std::vector<std::string> shader_path;

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

shader_store::shader_store(
    context& ctx,
    const std::vector<std::string>& shader_path
): ctx(&ctx), shader_path(shader_path) {}

shader_store::~shader_store()
{
}

multishader* shader_store::add(const shader::path& path)
{
    auto it = shaders.find(path);
    if(it != shaders.end())
        throw std::runtime_error(
            "Shader with source files [" + path.vert + ", " + path.frag +
            "] already exists!"
        );

    shader::path full_path{
        find_file(shader_path, path.vert),
        find_file(shader_path, path.frag)
    };
    multishader* s = new multishader(*ctx, full_path, shader_path); 
    shaders[path] = std::unique_ptr<multishader>(s);
    return s;
}

void shader_store::remove(const shader::path& path)
{
    shaders.erase(path);
}

multishader* shader_store::get(const shader::path& path)
{
    auto it = shaders.find(path);
    if(it == shaders.end())
        return add(path);
    return it->second.get();
}

size_t shader_store::size() const { return shaders.size(); }

shader_store::iterator shader_store::begin() { return shaders.begin(); }
shader_store::iterator shader_store::end() { return shaders.end(); }

shader_store::const_iterator shader_store::cbegin() const
{
    return shaders.cbegin(); 
}

shader_store::const_iterator shader_store::cend() const
{
    return shaders.cend();
}
