#ifndef RENDER_TARGET_HH
#define RENDER_TARGET_HH
#include "glheaders.hh"
#include "resource.hh"
#include "math.hh"

class render_target: public glresource
{
public:
    render_target(
        context& ctx,
        glm::uvec2 size = glm::uvec2(0)
    );
    virtual ~render_target();

    void bind(GLenum target = GL_FRAMEBUFFER);
    void unbind();
    bool is_bound(GLenum target = GL_FRAMEBUFFER) const;

    glm::uvec2 get_size() const;
    float get_aspect() const;

    GLuint get_fbo() const;

    static GLint get_current_read_fbo();
    static GLint get_current_write_fbo();
    static void reinstate_current_fbo();

protected:
    GLuint fbo;
    glm::uvec2 size;

    static GLint current_read_fbo;
    static GLint current_write_fbo;
};

#endif
