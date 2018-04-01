#include "clear_gbuffer.hh"
#include "gbuffer.hh"

method::clear_gbuffer::clear_gbuffer(gbuffer& gbuf): gbuf(&gbuf) {}
method::clear_gbuffer::~clear_gbuffer() {}

void method::clear_gbuffer::execute()
{
    gbuf->draw_all();
    gbuf->clear();
    gbuf->draw_lighting();
}

std::string method::clear_gbuffer::get_name() const
{
    return "clear_gbuffer";
}
