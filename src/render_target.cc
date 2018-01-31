#include "render_target.hh"

GLuint render_target::current_fbo = 0;

render_target::render_target(GLuint fbo, glm::uvec2 size)
: fbo(fbo), size(size) {}

render_target::~render_target() {}

void render_target::bind()
{
    if(current_fbo != fbo)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        current_fbo = fbo;
    }
}

void render_target::unbind()
{
    if(fbo == current_fbo)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        current_fbo = 0;
    }
}

bool render_target::is_bound() const
{
    return fbo == current_fbo;
}

glm::uvec2 render_target::get_size() const
{
    return size;
}

float render_target::get_aspect() const
{
    return size.x/(float)size.y;
}
