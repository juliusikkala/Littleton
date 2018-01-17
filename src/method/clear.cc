#include "clear.hh"
#include "glheaders.hh"

method::clear::clear(glm::vec4 color, double depth, int stencil)
: color(color), depth(depth), stencil(stencil) {}

method::clear::~clear(){}

class clear_impl: public pipeline_method
{
public:
    clear_impl(glm::vec4 color, double depth, int stencil)
    : color(color), depth(depth), stencil(stencil) {}

    void execute() override
    {
        glClearColor(color.r, color.g, color.b, color.a);
        glClearDepth(depth);
        glClearStencil(stencil);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    }

private:
    glm::vec4 color;
    double depth, stencil;
};

pipeline_method* method::clear::pipeline_build(pipeline&) const
{
    return new clear_impl(color, depth, stencil);
}

