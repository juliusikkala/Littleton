#include "render_target.hh"

GLint render_target::current_fbo = -1;

render_target::render_target(context& ctx, GLuint fbo, glm::uvec2 size)
: glresource(ctx), fbo(fbo), size(size) {}

render_target::~render_target() {}

void render_target::bind()
{
    if(fbo != (GLuint)current_fbo)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        current_fbo = fbo;
        glViewport(0, 0, size.x, size.y);
    }
}

void render_target::unbind()
{
    if(fbo == (GLuint)current_fbo)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        current_fbo = -1;
    }
}

bool render_target::is_bound() const
{
    return fbo == (GLuint)current_fbo;
}

glm::uvec2 render_target::get_size() const
{
    return size;
}

float render_target::get_aspect() const
{
    return size.x/(float)size.y;
}
