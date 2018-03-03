#include "resource.hh"

resource::~resource() {}
void resource::load() const {}
void resource::unload() const {}

glresource::glresource(context& ctx): ctx(&ctx) {}
context& glresource::get_context() const { return *ctx; }

