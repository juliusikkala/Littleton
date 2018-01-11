#ifndef TEXTURE_HH
#define TEXTURE_HH
#include "glheaders.hh"
#include "resources.hh"
#include <string>

class texture_data
{
public:
    texture_data(const char* path);
    ~texture_data();

    void load();
    void unload();

    GLuint texture;
    GLint internal_format;
    GLenum format, target, type;

private:
    std::string path;
};

class texture: resource<texture_data>
{
public:
    using resource<texture_data>::resource;
};

#endif
