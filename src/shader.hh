#ifndef SHADER_HH
#define SHADER_HH
#include "resources.hh"
#include "glheaders.hh"

class shader_data
{
public:
    shader_data(
        const std::string& vert_src,
        const std::string& frag_src
    );
    shader_data(const shader_data& other) = delete;
    ~shader_data();

    void load();
    void unload();

    std::string vert_src, frag_src;
    GLint program;
};

class shader: resource<shader_data>
{
public:
    shader(resource_manager& manager, const std::string& resource_name);
    ~shader();
};
#endif
