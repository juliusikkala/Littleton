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
        params(
            bool srgb = false,
            GLint interpolation = GL_LINEAR_MIPMAP_LINEAR,
            GLint extension = GL_REPEAT,
            unsigned anisotropy = 16,
            glm::vec4 border_color = glm::vec4(0,0,0,0)
        );

        bool srgb;
        GLint interpolation;
        GLint extension;
        unsigned anisotropy;
        glm::vec4 border_color;
    };

    static const params DEPTH_PARAMS;
    static const params SHADOW_MAP_PARAMS;

    texture(
        context& ctx,
        const std::string& path,
        const params& p = params(),
        GLenum target = GL_TEXTURE_2D
    );

    texture(
        context& ctx,
        glm::uvec2 size,
        GLenum external_format,
        GLint internal_format,
        GLenum type,
        const params& p = params(false, GL_LINEAR, GL_CLAMP_TO_EDGE, 0),
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

    static texture* create(
        context& ctx,
        const std::string& path,
        const params& p = params(),
        GLenum target = GL_TEXTURE_2D
    );

    static texture* create(
        context& ctx,
        glm::uvec2 size,
        GLenum external_format,
        GLint internal_format,
        GLenum type,
        const params& p = params(false, GL_LINEAR, GL_CLAMP_TO_EDGE, 0),
        GLenum target = GL_TEXTURE_2D,
        size_t data_size = 0,
        const void* data = nullptr
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
        glm::uvec2 size,
        GLenum external_format,
        GLint internal_format,
        GLenum type,
        const params& p,
        GLenum target,
        const void* data = nullptr
    ) const;

    void basic_unload() const;

    mutable GLuint tex;
    mutable GLint internal_format;
    mutable GLenum external_format, target, type;
    mutable glm::uvec2 size;
};

#endif
