#include "texture.hh"
#include "context.hh"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include <cstring>
#include "stb_image.h"

static GLint interpolation_without_mipmap(GLint interpolation)
{
    if(
        interpolation == GL_LINEAR_MIPMAP_LINEAR ||
        interpolation == GL_LINEAR_MIPMAP_NEAREST
    ) return GL_LINEAR; 
    else if(
        interpolation == GL_NEAREST_MIPMAP_NEAREST ||
        interpolation == GL_NEAREST_MIPMAP_LINEAR
    ) return GL_NEAREST; 
    else return interpolation;
}

texture::params::params(
    bool srgb,
    GLint interpolation,
    GLint extension,
    unsigned anisotropy,
    glm::vec4 border_color
): srgb(srgb), interpolation(interpolation), extension(extension),
   anisotropy(anisotropy), border_color(border_color)
{}

const texture::params texture::DEPTH_PARAMS(
    false,
    GL_NEAREST,
    GL_CLAMP_TO_BORDER,
    0,
    glm::vec4(1)
);

static void apply_params(
    context& ctx,
    GLenum target,
    GLenum& external_format,
    const texture::params& p,
    bool has_mipmaps = false
){
    glTexParameteri(
        target,
        GL_TEXTURE_MIN_FILTER,
        has_mipmaps ?
            p.interpolation :
            interpolation_without_mipmap(p.interpolation)
    );
    glTexParameteri(
        target,
        GL_TEXTURE_MAG_FILTER,
        interpolation_without_mipmap(p.interpolation)
    );

    if(external_format == GL_DEPTH_COMPONENT && p.interpolation == GL_LINEAR)
    {
        glTexParameteri(
            target,
            GL_TEXTURE_COMPARE_MODE,
            GL_COMPARE_REF_TO_TEXTURE
        );
    }

    glTexParameteri(target, GL_TEXTURE_WRAP_S, p.extension);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, p.extension);

    glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, (float*)&p.border_color);

    if(p.anisotropy != 0 && GLEW_EXT_texture_filter_anisotropic)
    {
        unsigned max_anisotropy = ctx[GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT];
        glTexParameterf(
            target,
            GL_TEXTURE_MAX_ANISOTROPY_EXT,
            std::min(p.anisotropy, max_anisotropy)
        );
    }
}

static unsigned choose_alignment(unsigned line_bytes)
{
    if((line_bytes & 7) == 0) return 8;
    if((line_bytes & 3) == 0) return 4;
    if((line_bytes & 1) == 0) return 2;
    return 1;
}

static GLuint load_texture(
    context& ctx,
    const std::string& path,
    const texture::params& p,
    GLenum target,
    GLint& internal_format,
    GLenum& external_format,
    GLenum& type,
    glm::uvec2& size
){
    int w = 0, h = 0, n = 0;
    bool hdr = stbi_is_hdr(path.c_str());
    void* data = nullptr;

    stbi_set_flip_vertically_on_load(1);

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
    size.x = w;
    size.y = h;

    if(!data)
    {
        throw std::runtime_error("Unable to read " + path);
    }
    int max_size = ctx[GL_MAX_TEXTURE_SIZE];
    if(w > max_size || h > max_size)
        throw std::runtime_error(
            "Texture is too large, maximum is "
            + std::to_string(max_size) + "x" + std::to_string(max_size)
            + " but texture is " + std::to_string(w) + "x"
            + std::to_string(h) + "."
        );

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
        if(p.srgb) internal_format = GL_SRGB8;
        else internal_format = hdr ? GL_RGB16 : GL_RGB8;
        external_format = GL_RGB;
        break;
    case 4:
        if(p.srgb) internal_format = GL_SRGB8_ALPHA8;
        else internal_format = hdr ? GL_RGBA16 : GL_RGBA8;
        external_format = GL_RGBA;
        break;
    }

    unsigned mipmap_count = floor(log2(std::max(w, h)))+1;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    GLint prev_tex = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_tex);
    glBindTexture(target, tex);

    glTexStorage2D(
        target,
        mipmap_count,
        internal_format,
        w, h
    );

    glPixelStorei(GL_UNPACK_ALIGNMENT, choose_alignment(w * n));

    glTexSubImage2D(
        target, 0, 0, 0, w, h,
        external_format, type,
        data
    );
    glGenerateMipmap(target);

    apply_params(ctx, target, external_format, p, true);

    if(prev_tex != 0) glBindTexture(target, prev_tex);

    if(glGetError() != GL_NO_ERROR)
        throw std::runtime_error("Failed to create texture from " + path);

    stbi_image_free(data);
    return tex;
}

texture::texture(context& ctx)
: glresource(ctx), tex(0), target(GL_TEXTURE_2D) {}

texture::texture(
    context& ctx,
    const std::string& path,
    const params& p,
    GLenum target
): glresource(ctx), tex(0), target(target)
{
    basic_load(path, p, target);
}

texture::texture(
    context& ctx,
    glm::uvec2 size,
    GLenum external_format,
    GLint internal_format,
    GLenum type,
    const params& p,
    GLenum target,
    const void* data
): glresource(ctx), tex(0), internal_format(internal_format),
   external_format(external_format), target(target), type(type)
{
    basic_load(size, external_format, internal_format, type, p, target, data);
}

texture::texture(texture&& other)
: glresource(other.get_context())
{
    other.load();

    tex = other.tex;
    internal_format = other.internal_format;
    external_format = other.external_format;
    target = other.target;
    type = other.type;
    size = other.size;

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

glm::uvec2 texture::get_size() const
{
    load();
    return size;
}

GLint texture::bind(unsigned index)
{
    load();
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(target, tex);
    return index;
}

class file_texture: public texture
{
public:
    file_texture(
        context& ctx,
        const std::string& path,
        const params& p,
        GLenum target
    ): texture(ctx), p(p), path(path)
    {
        this->target = target;
    }

    void load() const override
    {
        basic_load(path, p, target);
    }

    void unload() const override
    {
        basic_unload();
    }

private:
    params p;
    std::string path;
};

texture* texture::create(
    context& ctx,
    const std::string& path,
    const params& p,
    GLenum target
){
    return new file_texture(ctx, path, p, target);
}

class data_texture: public texture
{
public:
    data_texture(
        context& ctx,
        glm::uvec2 size,
        GLenum external_format,
        GLint internal_format,
        GLenum type,
        const params& p,
        GLenum target,
        size_t data_size,
        const void* data
    ): texture(ctx), p(p), size(size)
    {
        this->external_format = external_format;
        this->internal_format = internal_format;
        this->type = type;
        this->target = target;

        if(data)
        {
            this->data = new uint8_t[data_size];
            memcpy(this->data, data, data_size);
        }
    }

    ~data_texture()
    {
        if(data) delete [] (uint8_t*)data;
    }

    void load() const override
    {
        basic_load(
            size,
            external_format,
            internal_format,
            type,
            p,
            target,
            data
        );
    }

    void unload() const override
    {
        basic_unload();
    }

private:
    params p;
    glm::uvec2 size;
    void* data;
};

texture* texture::create(
    context& ctx,
    glm::uvec2 size,
    GLenum external_format,
    GLint internal_format,
    GLenum type,
    const params& p,
    GLenum target,
    size_t data_size,
    const void* data
){
    return new data_texture(
        ctx, size, external_format, internal_format, type, p, target,
        data_size, data
    );
}

void texture::basic_load(
    const std::string& path,
    const params& p,
    GLenum target
) const
{
    if(tex) return;

    tex = load_texture(
        get_context(),
        path,
        p,
        target,
        internal_format,
        external_format,
        type,
        size
    );

    if(tex == 0)
    {
        throw std::runtime_error("Unable to read texture " + path);
    }
}

void texture::basic_load(
    glm::uvec2 size,
    GLenum external_format,
    GLint internal_format,
    GLenum type,
    const params& p,
    GLenum target,
    const void* data
) const {
    if(tex) return;

    glGenTextures(1, &tex);
    glBindTexture(target, tex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if(target == GL_TEXTURE_2D)
    {
        glTexImage2D(
            target,
            0,
            internal_format,
            size.x,
            size.y,
            0,
            external_format,
            type,
            data
        );
    }
    else if(target == GL_TEXTURE_1D)
    {
        glTexImage1D(
            target,
            0,
            internal_format,
            size.x,
            0,
            external_format,
            type,
            data
        );
    }
    else
    {
        throw std::runtime_error("Unknown texture target!");
    }

    apply_params(get_context(), target, external_format, p);

    if(glGetError() != GL_NO_ERROR)
        throw std::runtime_error("Failed to create empty texture");

    this->external_format = external_format;
    this->internal_format = internal_format;
    this->type = type;
    this->target = target;
    this->size = size;
}

void texture::basic_unload() const
{
    if(tex != 0)
    {
        glDeleteTextures(1, &tex);
        tex = 0;
    }
}
