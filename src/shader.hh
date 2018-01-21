#ifndef SHADER_HH
#define SHADER_HH
#include "resources.hh"
#include "glheaders.hh"

class shader: public resource
{
public:
    shader();
    shader(
        const std::string& vert_src,
        const std::string& frag_src
    );
    ~shader();

    GLuint get_program() const;

    static shader* create(
        const std::string& vert_src,
        const std::string& frag_src
    );

protected:
    void basic_load(
        const std::string& vert_src,
        const std::string& frag_src
    ) const;
    void basic_unload() const;

    mutable GLuint program;
};

#endif
