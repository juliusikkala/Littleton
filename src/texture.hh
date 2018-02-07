#ifndef TEXTURE_HH
#define TEXTURE_HH
#include "glheaders.hh"
#include "resources.hh"
#include <string>
#include <glm/glm.hpp>

class texture: public resource, public glresource
{
public:
    struct params
    {
        bool srgb = false;
        GLint interpolation = GL_LINEAR_MIPMAP_LINEAR;
        GLint extension = GL_REPEAT;
    };

    texture(
        context& ctx,
        const std::string& path,
        const params& p,
        GLenum target = GL_TEXTURE_2D
    );

    texture(
        context& ctx,
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
    glm::uvec2 get_size() const;

    static texture* create(
        context& ctx,
        const std::string& path,
        const params& p,
        GLenum target = GL_TEXTURE_2D
    );

    static texture* create(
        context& ctx,
        unsigned w,
        unsigned h,
        GLenum external_format,
        GLint internal_format,
        GLenum type
    );

    // Returns the index
    GLint bind(unsigned index = 0);

protected:
    texture(context& ctx);

    void basic_load(
        const std::string& path,
        const params& p,
        GLenum target
    ) const;
    void basic_load(
        unsigned w,
        unsigned h,
        GLenum external_format,
        GLint internal_format,
        GLenum type,
        GLenum target
    ) const;
    void basic_unload() const;

    mutable GLuint tex;
    mutable GLint internal_format;
    mutable GLenum external_format, target, type;
    mutable glm::uvec2 size;
};

#endif
