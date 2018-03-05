#include "blit_framebuffer.hh"

method::blit_framebuffer::blit_framebuffer(
    render_target& dst,
    render_target& src,
    blit_type type
): target_method(dst), src(&src), type(type) {}

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
    target_method::execute();

    render_target& dst = get_target();

    src->bind(GL_READ_FRAMEBUFFER);
    dst.bind(GL_DRAW_FRAMEBUFFER);

    glm::uvec2 src_size = src->get_size();
    glm::uvec2 dst_size = dst.get_size();

    glBlitFramebuffer(
        0, 0, src_size.x, src_size.y,
        0, 0, dst_size.x, dst_size.y,
        type,
        type == COLOR_ONLY ? GL_LINEAR : GL_NEAREST
    );
}

std::string method::blit_framebuffer::get_name() const
{
    return "blit_framebuffer";
}
