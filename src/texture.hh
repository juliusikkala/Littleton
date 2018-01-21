#ifndef TEXTURE_HH
#define TEXTURE_HH
#include "glheaders.hh"
#include "resources.hh"
#include <string>

class texture: public resource
{
public:
    texture();
    texture(const std::string& path);
    texture(
        unsigned w,
        unsigned h,
        GLenum external_format,
        GLint internal_format,
        GLenum type
    );
    texture(const texture& other) = delete;
    texture(texture&& other);
    ~texture();

    GLuint get_texture() const;
    GLint get_internal_format() const;
    GLenum get_external_format() const;
    GLenum get_target() const;
    GLenum get_type() const;

    static texture* create(const std::string& path);
    static texture* create(
        unsigned w,
        unsigned h,
        GLenum external_format,
        GLint internal_format,
        GLenum type
    );

protected:
    void basic_load(const std::string& path) const;
    void basic_load(
        unsigned w,
        unsigned h,
        GLenum external_format,
        GLint internal_format,
        GLenum type
    ) const;
    void basic_unload() const;

    mutable GLuint tex;
    mutable GLint internal_format;
    mutable GLenum external_format, target, type;
};

#endif
