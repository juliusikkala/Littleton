#ifndef RENDER_TARGET_HH
#define RENDER_TARGET_HH
#include "glheaders.hh"
#include "resources.hh"
#include <glm/glm.hpp>

class render_target: public glresource
{
public:
    render_target(
        context& ctx,
        glm::uvec2 size = glm::uvec2(0)
    );
    virtual ~render_target();

    virtual void bind();
    virtual void unbind();
    bool is_bound() const;

    glm::uvec2 get_size() const;
    float get_aspect() const;

    GLuint get_fbo() const;
    static GLint get_current_fbo();

protected:
    GLuint fbo;
    glm::uvec2 size;

    static GLint current_fbo;
};

#endif
