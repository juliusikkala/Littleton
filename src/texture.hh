#ifndef TEXTURE_HH
#define TEXTURE_HH
#include "glheaders.hh"
#include "resources.hh"
#include <string>

class texture
{
public:
    texture(const std::string& path);
    texture(
        unsigned w,
        unsigned h,
        GLint external_format,
        GLint internal_format,
        GLenum type
    );
    ~texture();

    GLuint get_texture() const;
    GLint get_internal_format() const;
    GLenum get_external_format() const;
    GLenum get_target() const;
    GLenum get_type() const;

private:
    GLuint tex;
    GLint internal_format;
    GLenum format, target, type;
};

using texture_ptr = resource_ptr<texture>;

#endif
