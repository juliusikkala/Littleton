#include "texture.hh"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include "stb_image.h"

static GLuint load_texture(
    const std::string& path,
    GLenum target,
    GLint& internal_format,
    GLenum& external_format,
    GLenum& type
){
    int w = 0, h = 0, n = 0;
    bool hdr = stbi_is_hdr(path.c_str());
    void* data = nullptr;

    if(hdr)
    {
        data = stbi_loadf(path.c_str(), &w, &h, &n, 0);
        type = GL_FLOAT;
    }
    else
    {
        data = stbi_load(path.c_str(), &w, &h, &n, 0);
        type = GL_UNSIGNED_BYTE;
    }

    if(!data)
    {
        throw std::runtime_error("Unable to read " + path);
    }

    switch(n)
    {
    case 1:
        internal_format = hdr ? GL_R16 : GL_R8;
        external_format = GL_RED;
        break;
    case 2:
        internal_format = hdr ? GL_RG16 : GL_RG8;
        external_format = GL_RG;
        break;
    case 3:
        internal_format = hdr ? GL_RGB16 : GL_RGB8;
        external_format = GL_RGB;
        break;
    case 4:
        internal_format = hdr ? GL_RGBA16 : GL_RGBA8;
        external_format = GL_RGBA;
        break;
    }

    unsigned mipmap_count = floor(log2(std::max(w, h)))+1;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    GLint prev_tex;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_tex);
    glBindTexture(target, tex);

    glTexStorage2D(
        target,
        mipmap_count,
        internal_format,
        w, h
    );

    glTexSubImage2D(
        target, 0, 0, 0, w, h,
        external_format, type,
        data
    );
    glGenerateMipmap(target);

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if(prev_tex != 0) glBindTexture(target, prev_tex);

    stbi_image_free(data);
    return tex;
}

texture::texture()
: tex(0), target(GL_TEXTURE_2D) {}

texture::texture(const std::string& path, GLenum target)
: tex(0), target(target)
{
    tex = load_texture(
        path,
        target,
        internal_format,
        external_format,
        type
    );
    if(tex == 0)
    {
        throw std::runtime_error("Unable to read texture " + path);
    }
}

texture::texture(
    unsigned w,
    unsigned h,
    GLenum external_format,
    GLint internal_format,
    GLenum type,
    GLenum target
): tex(0), internal_format(internal_format), external_format(external_format),
   target(target), type(type)
{
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, w, h);
}

texture::texture(texture&& other)
{
    other.load();

    tex = other.tex;
    internal_format = other.internal_format;
    external_format = other.external_format;
    target = other.target;
    type = other.type;

    other.tex = 0;
}

texture::~texture()
{
    basic_unload();
}

GLuint texture::get_texture() const
{
    load();
    return tex;
}

GLint texture::get_internal_format() const
{
    load();
    return internal_format;
}

GLenum texture::get_external_format() const
{
    load();
    return external_format;
}

GLenum texture::get_target() const
{
    load();
    return target;
}

GLenum texture::get_type() const
{
    load();
    return type;
}

void texture::bind(unsigned index)
{
    load();
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(target, tex);
}

class file_texture: public texture
{
public:
    file_texture(const std::string& path, GLenum target)
    : path(path)
    {
        this->target = target;
    }

    void load() const override
    {
        basic_load(path, target);
    }

    void unload() const override
    {
        basic_unload();
    }
private:
    std::string path;
};

texture* texture::create(const std::string& path, GLenum target)
{
    return new file_texture(path, target);
}

class empty_texture: public texture
{
public:
    empty_texture(
        unsigned w,
        unsigned h,
        GLenum external_format,
        GLint internal_format,
        GLenum type,
        GLenum target
    ): w(w), h(h)
    {
        this->external_format = external_format;
        this->internal_format = internal_format;
        this->type = type;
        this->target = target;
    }

    void load() const override
    {
        basic_load(w, h, external_format, internal_format, type, target);
    }

    void unload() const override
    {
        basic_unload();
    }

private:
    unsigned w, h;
};

texture* texture::create(
    unsigned w,
    unsigned h,
    GLenum external_format,
    GLint internal_format,
    GLenum type,
    GLenum target
){
    return new empty_texture(
        w, h, external_format, internal_format, type, target
    );
}

void texture::basic_load(const std::string& path, GLenum target) const
{
    if(tex) return;

    tex = load_texture(
        path,
        target,
        internal_format,
        external_format,
        type
    );

    if(tex == 0)
    {
        throw std::runtime_error("Unable to read texture " + path);
    }
}

void texture::basic_load(
    unsigned w,
    unsigned h,
    GLenum external_format,
    GLint internal_format,
    GLenum type,
    GLenum target
) const {
    if(tex) return;

    glGenTextures(1, &tex);
    glBindTexture(target, tex);
    glTexStorage2D(target, 1, internal_format, w, h);
}

void texture::basic_unload() const
{
    if(tex != 0)
    {
        glDeleteTextures(1, &tex);
        tex = 0;
    }
}
