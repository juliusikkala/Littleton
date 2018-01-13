#ifndef SHADER_HH
#define SHADER_HH
#include "resources.hh"
#include "glheaders.hh"

class shader
{
public:
    shader(
        const std::string& vert_src,
        const std::string& frag_src
    );
    ~shader();

    GLuint get_program() const;

private:
    GLuint program;
};

using shader_ptr = resource_ptr<shader>;

#endif
