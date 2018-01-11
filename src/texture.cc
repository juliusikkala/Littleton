#include "texture.hh"
#include <gli/gli.hpp>
#include <glm/glm.hpp>

texture_data::texture_data(const char* path)
: texture(0), path(path)
{}

texture_data::~texture_data()
{
    unload();
}

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

void texture_data::load()
{
    if(texture == 0)
    {
        texture = load_texture(
            path,
            internal_format,
            format,
            target,
            type
        );
        if(texture == 0)
        {
            throw std::runtime_error("Unable to read texture " + path);
        }
    }
}

void texture_data::unload()
{
    if(texture != 0)
    {
        glDeleteTextures(1, &texture);
        texture = 0;
    }
}
