#ifndef FRAMEBUFFER_HH
#define FRAMEBUFFER_HH
#include "render_target.hh"
#include <vector>

class texture;
class framebuffer: public render_target
{
public:
    framebuffer(
        glm::uvec2 size,
        std::vector<texture*>&& targets = {},
        GLenum depth_stencil_format = GL_DEPTH24_STENCIL8
    );
    framebuffer(framebuffer&& f);
    ~framebuffer();

    void set_target(texture* target, unsigned index = 0);
    void remove_target(unsigned index = 0);

    void bind() override;

private:
    std::vector<texture*> targets;
    GLuint depth_stencil;
};

#endif
