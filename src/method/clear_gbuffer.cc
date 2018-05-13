#include "clear_gbuffer.hh"
#include "gbuffer.hh"

namespace lt::method
{

clear_gbuffer::clear_gbuffer(gbuffer& gbuf): gbuf(&gbuf) {}
clear_gbuffer::~clear_gbuffer() {}

void clear_gbuffer::execute()
{
    gbuf->set_draw(gbuffer::DRAW_ALL);
    gbuf->clear();
    gbuf->set_draw(gbuffer::DRAW_LIGHTING);
}

std::string clear_gbuffer::get_name() const
{
    return "clear_gbuffer";
}

} // namespace lt::method
