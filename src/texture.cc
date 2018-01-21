#include "texture.hh"
#include <gli/gli.hpp>
#include <glm/glm.hpp>

static GLuint load_texture(
    const std::string& path,
    GLint& internal_format,
    GLenum& external_format,
    GLenum& target,
    GLenum& type
){
    gli::gl gl(gli::gl::PROFILE_GL33);
    gli::texture texture = gli::load(path);
    if(texture.empty()) return 0;

    gli::gl::format format = gl.translate(
        texture.format(),
        texture.swizzles()
    );
    internal_format = format.Internal;
    external_format = format.External;
    type = format.Type;
    target = gl.translate(texture.target());
    glm::tvec3<GLsizei> extent = texture.extent();

    GLuint gl_tex = 0;

    glGenTextures(1, &gl_tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(target, gl_tex);
    glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, texture.levels() - 1);
    glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, format.Swizzles[0]);
    glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, format.Swizzles[1]);
    glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, format.Swizzles[2]);
    glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, format.Swizzles[3]);

    switch(target)
    {
    case GL_TEXTURE_1D:
        glTexStorage1D(target, texture.levels(), internal_format, extent.x);
        break;
    case GL_TEXTURE_1D_ARRAY:
    case GL_TEXTURE_2D:
    case GL_TEXTURE_CUBE_MAP:
    case GL_TEXTURE_RECTANGLE:
        glTexStorage2D(
            target,
            texture.levels(),
            internal_format,
            extent.x,
            target == GL_TEXTURE_1D_ARRAY ? texture.layers() : extent.y
        );
        break;
    case GL_TEXTURE_2D_ARRAY:
    case GL_TEXTURE_3D:
    case GL_TEXTURE_CUBE_MAP_ARRAY:
        glTexStorage3D(
            target,
            texture.levels(),
            internal_format,
            extent.x,
            extent.y,
            target == GL_TEXTURE_3D ?
                extent.z : texture.layers() * texture.faces()
        );
        break;
    default:
        glDeleteTextures(1, &gl_tex);
        return 0;
    }

    for(size_t layer = 0; layer < texture.layers(); ++layer)
    {
        for(size_t face = 0; face < texture.faces(); ++face)
        {
            for(size_t level = 0; level < texture.levels(); ++level)
            {
                extent = texture.extent(level);
                GLenum face_target = gli::is_target_cube(texture.target()) ?
                    face + GL_TEXTURE_CUBE_MAP_POSITIVE_X : target;

                switch(target)
                {
                case GL_TEXTURE_1D:
                    if(gli::is_compressed(texture.format()))
                    {
                        glCompressedTexSubImage1D(
                            face_target, level,
                            0, extent.x,
                            internal_format, texture.size(level),
                            texture.data(layer, face, level)
                        );
                    }
                    else
                    {
                        glTexSubImage1D(
                            face_target, level,
                            0, extent.x,
                            external_format, type,
                            texture.data(layer, face, level)
                        );
                    }
                    break;
                case GL_TEXTURE_1D_ARRAY:
                case GL_TEXTURE_2D:
                case GL_TEXTURE_CUBE_MAP:
                case GL_TEXTURE_RECTANGLE:
                    if(gli::is_compressed(texture.format()))
                    {
                        glCompressedTexSubImage2D(
                            face_target, level,
                            0, 0, extent.x,
                            target == GL_TEXTURE_1D_ARRAY ? layer : extent.y,
                            internal_format, texture.size(level),
                            texture.data(layer, face, level)
                        );
                    }
                    else
                    {
                        glTexSubImage2D(
                            face_target, level,
                            0, 0, extent.x,
                            target == GL_TEXTURE_1D_ARRAY ? layer : extent.y,
                            external_format, type,
                            texture.data(layer, face, level)
                        );
                    }
                    break;
                case GL_TEXTURE_2D_ARRAY:
                case GL_TEXTURE_3D:
                case GL_TEXTURE_CUBE_MAP_ARRAY:
                    if(gli::is_compressed(texture.format()))
                    {
                        glCompressedTexSubImage3D(
                            face_target, level,
                            0, 0, 0, extent.x, extent.y,
                            target == GL_TEXTURE_3D ? extent.z : layer,
                            internal_format, texture.size(level),
                            texture.data(layer, face, level)
                        );
                    }
                    else
                    {
                        glTexSubImage3D(
                            face_target, level,
                            0, 0, 0, extent.x, extent.y,
                            target == GL_TEXTURE_3D ? extent.z : layer,
                            external_format, type,
                            texture.data(layer, face, level)
                        );
                    }
                    break;
                default:
                    glDeleteTextures(1, &gl_tex);
                    return 0;
                }
            }
        }
    }

    return gl_tex;
}

texture::texture()
: tex(0) {}

texture::texture(const std::string& path)
: tex(0)
{
    tex = load_texture(
        path,
        internal_format,
        external_format,
        target,
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
    GLenum type
): tex(0), internal_format(internal_format), external_format(external_format),
   target(GL_TEXTURE_2D), type(type)
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

class file_texture: public texture
{
public:
    file_texture(const std::string& path)
    : path(path) { }

    void load() const override
    {
        basic_load(path);
    }

    void unload() const override
    {
        basic_unload();
    }
private:
    std::string path;
};

texture* texture::create(const std::string& path)
{
    return new file_texture(path);
}

class empty_texture: public texture
{
public:
    empty_texture(
        unsigned w,
        unsigned h,
        GLenum external_format,
        GLint internal_format,
        GLenum type
    ): w(w), h(h)
    {
        this->external_format = external_format;
        this->internal_format = internal_format;
        this->type = type;
        this->target = GL_TEXTURE_2D;
    }

    void load() const override
    {
        basic_load(w, h, external_format, internal_format, type);
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
    GLenum type
){
    return new empty_texture(w, h, external_format, internal_format, type);
}

void texture::basic_load(const std::string& path) const
{
    if(tex) return;

    tex = load_texture(
        path,
        internal_format,
        external_format,
        target,
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
    GLenum type
) const {
    if(tex) return;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, internal_format, w, h);
}

void texture::basic_unload() const
{
    if(tex != 0)
    {
        glDeleteTextures(1, &tex);
        tex = 0;
    }
}
