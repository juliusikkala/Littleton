#ifndef FRAMEBUFFER_HH
#define FRAMEBUFFER_HH
#include "render_target.hh"
#include <vector>

class texture;
class framebuffer: public render_target
{
public:
    framebuffer(
        context& ctx,
        glm::uvec2 size,
        std::vector<texture*>&& targets = {},
        GLenum depth_stencil_format = 0
    );

    framebuffer(
        context& ctx,
        glm::uvec2 size,
        std::vector<texture*>&& targets,
        texture* depth_stencil_target
    );
    framebuffer(framebuffer&& f);
    ~framebuffer();

    void set_target(texture* target, unsigned index = 0);
    void remove_target(unsigned index = 0);

    void set_depth_target(texture* target);
    void remove_depth_target();

    void bind(GLenum target = GL_FRAMEBUFFER) override;

private:
    std::vector<texture*> color_targets;
    texture* depth_stencil_target;
    GLuint depth_stencil_rbo;
};

#endif
