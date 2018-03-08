#include "resource_pool.hh"

resource_pool::container::~container() {};

resource_pool::resource_pool(context& ctx): glresource(ctx) { }
resource_pool::~resource_pool() { }
