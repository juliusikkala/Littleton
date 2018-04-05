#include "clear_gbuffer.hh"
#include "gbuffer.hh"

method::clear_gbuffer::clear_gbuffer(gbuffer& gbuf): gbuf(&gbuf) {}
method::clear_gbuffer::~clear_gbuffer() {}

void method::clear_gbuffer::execute()
{
    gbuf->set_draw(gbuffer::DRAW_ALL);
    gbuf->clear();
    gbuf->set_draw(gbuffer::DRAW_LIGHTING);
}

std::string method::clear_gbuffer::get_name() const
{
    return "clear_gbuffer";
}
