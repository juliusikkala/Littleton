#ifndef TEXTURE_HH
#define TEXTURE_HH
#include "glheaders.hh"
#include "resource.hh"
#include <string>
#include "math.hh"

class texture: public resource, public glresource
{
public:
    texture(
        context& ctx,
        const std::string& path,
        bool srgb = false,
        GLenum target = GL_TEXTURE_2D
    );

    texture(
        context& ctx,
        glm::uvec2 size,
        GLint format,
        GLenum type,
        unsigned samples = 0,
        GLenum target = GL_TEXTURE_2D,
        const void* data = nullptr
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

    void generate_mipmaps();

    static texture* create(
        context& ctx,
        const std::string& path,
        bool srgb = false,
        GLenum target = GL_TEXTURE_2D
    );

    static texture* create(
        context& ctx,
        glm::uvec2 size,
        GLint internal_format,
        GLenum type,
        unsigned samples = 0,
        GLenum target = GL_TEXTURE_2D,
        size_t data_size = 0,
        const void* data = nullptr
    );

protected:
    texture(context& ctx);

    void basic_load(
        const std::string& path,
        bool srgb,
        GLenum target
    ) const;

    void basic_load(
        glm::uvec2 size,
        GLint internal_format,
        GLenum type,
        unsigned samples,
        GLenum target,
        const void* data = nullptr
    ) const;

    void basic_unload() const;

    mutable GLuint tex;
    mutable GLint internal_format;
    mutable GLint target;
    mutable GLenum type;
    mutable glm::uvec2 size;
};

#endif
