#include "sampler.hh"
#include "context.hh"
#include <algorithm>

namespace lt
{

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

sampler::sampler(
    context& ctx,
    GLint interpolation_mag,
    GLint interpolation_min,
    GLint extension,
    unsigned anisotropy,
    glm::vec4 border_color,
    GLint comparison_mode
): glresource(ctx)
{
    glGenSamplers(1, &sampler_object);

    set_interpolation(interpolation_mag, interpolation_min);
    set_extension(extension);
    set_anisotropy(anisotropy);
    set_border_color(border_color);
    set_comparison_mode(comparison_mode);
}

sampler::~sampler()
{
    glDeleteSamplers(1, &sampler_object);
}

void sampler::set_interpolation(
    GLint interpolation_mag,
    GLint interpolation_min
){
    glSamplerParameteri(
        sampler_object,
        GL_TEXTURE_MAG_FILTER,
        interpolation_without_mipmap(interpolation_mag)
    );

    glSamplerParameteri(
        sampler_object,
        GL_TEXTURE_MIN_FILTER,
        interpolation_min
    );
}

void sampler::set_extension(GLint extension)
{
    glSamplerParameteri(sampler_object, GL_TEXTURE_WRAP_S, extension);
    glSamplerParameteri(sampler_object, GL_TEXTURE_WRAP_T, extension);
    glSamplerParameteri(sampler_object, GL_TEXTURE_WRAP_R, extension);
}

void sampler::set_anisotropy(unsigned anisotropy)
{
    if(GLEW_EXT_texture_filter_anisotropic)
    {
        unsigned max_anisotropy =
            get_context()[GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT];

        glSamplerParameterf(
            sampler_object,
            GL_TEXTURE_MAX_ANISOTROPY_EXT,
            std::clamp(anisotropy, 1u, max_anisotropy)
        );
    }
}

void sampler::set_border_color(glm::vec4 border_color)
{
    glSamplerParameterfv(
        sampler_object,
        GL_TEXTURE_BORDER_COLOR,
        (float*)&border_color
    );
}

void sampler::set_comparison_mode(GLint comparison_mode)
{
    glSamplerParameteri(
        sampler_object,
        GL_TEXTURE_COMPARE_MODE,
        comparison_mode
    );
}

GLint sampler::bind(const texture& tex, unsigned index) const
{
    return bind(tex.get_texture(), index, tex.get_target());
}

GLint sampler::bind(GLuint tex, unsigned index, GLenum target) const
{
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(target, tex);
    glBindSampler(index, sampler_object);
    return index;
}

} // namespace lt
