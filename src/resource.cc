#include "resource.hh"

resource::resource(): loaded(false) {}
resource::~resource() {}

void resource::load() const
{
    if(!loaded)
    {
        load_impl();
        loaded = true;
    }
}

void resource::unload() const
{
    if(loaded)
    {
        unload_impl();
        loaded = false;
    }
}

void resource::load_impl() const { }

void resource::unload_impl() const { }

glresource::glresource(context& ctx): ctx(&ctx) {}
context& glresource::get_context() const { return *ctx; }

