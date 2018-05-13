#include "stencil_handler.hh"
#include "glheaders.hh"

namespace lt
{

stencil_handler::stencil_handler(): value(1), ref(1) {}

void stencil_handler::set_stencil_draw(unsigned value)
{
    this->value = value;
}

void stencil_handler::set_stencil_cull(unsigned ref)
{
    this->ref = ref;
}

void stencil_handler::stencil_disable()
{
    glDisable(GL_STENCIL_TEST);
}

void stencil_handler::stencil_draw()
{
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, value, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);
}

void stencil_handler::stencil_cull()
{
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, ref, 0xFF);
    glStencilMask(0x00);
}

} // namespace lt
