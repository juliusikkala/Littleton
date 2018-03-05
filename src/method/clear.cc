#include "clear.hh"
#include "glheaders.hh"

method::clear::clear(
    render_target& target,
    glm::vec4 color,
    double depth,
    int stencil
): target_method(target), color(color), depth(depth), stencil(stencil) {}

method::clear::~clear(){}

void method::clear::execute()
{
    target_method::execute();

    glStencilMask(0xFF);
    glClearColor(color.r, color.g, color.b, color.a);
    glClearDepth(depth);
    glClearStencil(stencil);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
}

std::string method::clear::get_name() const
{
    return "clear";
}
