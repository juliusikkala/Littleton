#ifndef DOUBLEBUFFER_HH
#define DOUBLEBUFFER_HH
#include "resources.hh"
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
    };

    target& input();
    const target& input() const;

    texture& output();
    const texture& output() const;

    void swap();

private:
    unsigned cur_index;

    texture buffers[2];
    target targets[2];
};

#endif

