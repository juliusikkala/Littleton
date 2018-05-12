#include "resource.hh"

resource::resource(): loaded(false), do_unload(false), references(0) {}
resource::~resource() {}

void resource::load() const
{
    if(!loaded)
    {
        load_impl();
        loaded = true;
    }
    do_unload = false;
}

void resource::unload() const
{
    if(loaded)
    {
        if(references)
        {
            do_unload = true;
        }
        else
        {
            unload_impl();
            do_unload = false;
            loaded = false;
        }
    }
}

void resource::force_unload() const
{
    references = 0;
    unload();
}

void resource::load_impl() const { }

void resource::unload_impl() const { }

void resource::link() const
{
    references++;
}

void resource::unlink() const
{
    if(references) references--;
    if(do_unload && references == 0)
    {
        unload();
    }
}

glresource::glresource(context& ctx): ctx(&ctx) {}
context& glresource::get_context() const { return *ctx; }

