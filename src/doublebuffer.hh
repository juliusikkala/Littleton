#ifndef DOUBLEBUFFER_HH
#define DOUBLEBUFFER_HH
#include "resource.hh"
#include "render_target.hh"
#include "texture.hh"

class doublebuffer: public glresource
{
public:
    doublebuffer(
        context& ctx,
        glm::uvec2 size,
        GLenum external_format,
        GLint internal_format,
        GLenum type
    );
    doublebuffer(doublebuffer&& other);
    ~doublebuffer();

    class target: public render_target
    {
    public:
        target(context& ctx, texture& tex);
        target(target&& other);
        ~target();

        void set_depth_stencil(texture* depth_stencil);
    };

    target& input(unsigned index = 0);
    const target& input(unsigned index = 0) const;

    texture& output(unsigned index = 0);
    const texture& output(unsigned index = 0) const;

    void set_depth_stencil(texture* depth_stencil);
    void set_depth_stencil(unsigned index, texture* depth_stencil);

    void swap();

private:
    unsigned cur_index;

    texture buffers[2];
    target targets[2];
};

#endif

