#include "render_target.hh"
#include <stdexcept>

namespace lt
{

GLint render_target::current_read_fbo = -1;
GLint render_target::current_write_fbo = -1;

render_target::render_target(context& ctx, glm::uvec2 size)
: glresource(ctx), fbo(0), size(size) {}

render_target::~render_target() {}

void render_target::bind(GLenum target)
{
    switch(target)
    {
    case GL_FRAMEBUFFER:
        if(fbo != (GLuint)current_read_fbo || fbo != (GLuint)current_write_fbo)
        {
            glBindFramebuffer(target, fbo);
            current_read_fbo = fbo;
            current_write_fbo = fbo;
        }
        break;
    case GL_READ_FRAMEBUFFER:
        if(fbo != (GLuint)current_read_fbo)
        {
            glBindFramebuffer(target, fbo);
            current_read_fbo = fbo;
        }
        break;
    case GL_DRAW_FRAMEBUFFER:
        if(fbo != (GLuint)current_write_fbo)
        {
            glBindFramebuffer(target, fbo);
            current_write_fbo = fbo;
        }
        break;
    default:
        throw std::runtime_error("Unknown render_target bind target");
    }
    glViewport(0, 0, size.x, size.y);
}

void render_target::unbind()
{
    if(fbo == (GLuint)current_read_fbo)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        current_read_fbo = -1;
    }

    if(fbo == (GLuint)current_write_fbo)
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        current_write_fbo = -1;
    }
}

bool render_target::is_bound(GLenum target) const
{
    switch(target)
    {
    case GL_FRAMEBUFFER:
        return fbo == (GLuint)current_read_fbo &&
               fbo == (GLuint)current_write_fbo;
    case GL_READ_FRAMEBUFFER:
        return fbo == (GLuint)current_read_fbo;
    case GL_DRAW_FRAMEBUFFER:
        return fbo == (GLuint)current_write_fbo;
    default:
        throw std::runtime_error("Unknown render_target unbind target");
    }
    return false;
}

glm::uvec2 render_target::get_size() const
{
    return size;
}

float render_target::get_aspect() const
{
    return size.x/(float)size.y;
}

GLuint render_target::get_fbo() const
{
    return fbo;
}

GLint render_target::get_current_read_fbo()
{
    return current_read_fbo;
}

GLint render_target::get_current_write_fbo()
{
    return current_write_fbo;
}

void render_target::reinstate_current_fbo()
{
    if(current_read_fbo != -1)
        glBindFramebuffer(GL_READ_FRAMEBUFFER, current_read_fbo);

    if(current_write_fbo != -1)
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, current_write_fbo);
}

} // namespace lt
