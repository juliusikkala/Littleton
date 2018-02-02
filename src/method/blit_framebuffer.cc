#include "blit_framebuffer.hh"

method::blit_framebuffer::blit_framebuffer(
    render_target& dst,
    render_target& src,
    blit_type type
): pipeline_method(dst), src(&src), type(type) {}

void method::blit_framebuffer::set_blit_type(blit_type type)
{
    this->type = type;    
}

void method::blit_framebuffer::set_src(render_target& src)
{
    this->src = &src;
}

void method::blit_framebuffer::execute()
{
    GLint cur = render_target::get_current_fbo();

    render_target& dst = get_target();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, src->get_fbo());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst.get_fbo());

    glm::uvec2 src_size = src->get_size();
    glm::uvec2 dst_size = dst.get_size();

    glBlitFramebuffer(
        0, 0, src_size.x, src_size.y,
        0, 0, dst_size.x, dst_size.y,
        type,
        type == COLOR_ONLY ? GL_LINEAR : GL_NEAREST
    );
    if(glGetError() != GL_NO_ERROR) throw std::runtime_error("I can't believe you've done this");

    if(cur != -1) glBindFramebuffer(GL_FRAMEBUFFER, cur);
}
