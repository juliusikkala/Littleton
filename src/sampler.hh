#ifndef SAMPLER_HH
#define SAMPLER_HH
#include "glheaders.hh"
#include "resource.hh"
#include <glm/glm.hpp>

class sampler: public glresource
{
public:
    sampler(
        context& ctx,
        GLint interpolation_mag = GL_LINEAR,
        GLint interpolation_min = GL_LINEAR_MIPMAP_LINEAR,
        GLint extension = GL_REPEAT,
        unsigned anisotropy = 16,
        glm::vec4 border_color = glm::vec4(0,0,0,0),
        GLint comparison_mode = GL_NONE
    );
    ~sampler();

    void set_interpolation(
        GLint interpolation_mag,
        GLint interpolation_min = GL_LINEAR_MIPMAP_LINEAR
    );
    void set_extension(GLint extension);
    void set_anisotropy(unsigned anisotropy);
    void set_border_color(glm::vec4 border_color);
    void set_comparison_mode(GLint comparison_mode);

    // Returns the index
    GLint bind(unsigned index = 0) const;

private:
    GLuint sampler_object;
};


#endif
