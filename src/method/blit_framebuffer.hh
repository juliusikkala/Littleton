#ifndef LT_METHOD_BLIT_FRAMEBUFFER_HH
#define LT_METHOD_BLIT_FRAMEBUFFER_HH
#include "pipeline.hh"
#include "render_target.hh"

namespace lt::method
{

class blit_framebuffer: public target_method
{
public:
    enum blit_type
    {
        COLOR_ONLY = GL_COLOR_BUFFER_BIT,
        DEPTH_ONLY = GL_DEPTH_BUFFER_BIT,
        STENCIL_ONLY = GL_STENCIL_BUFFER_BIT,
        DEPTH_STENCIL = GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
        COLOR_DEPTH = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
        COLOR_STENCIL = GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
        ALL = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
              GL_STENCIL_BUFFER_BIT
    };

    blit_framebuffer(
        render_target& dst,
        render_target& src,
        blit_type type = ALL
    );

    void set_blit_type(blit_type type);
    void set_src(render_target& src);

    void execute() override;

    std::string get_name() const override;

private:
    render_target* src;
    blit_type type;
};

} // namespace lt::method

#endif
