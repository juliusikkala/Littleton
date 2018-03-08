#include "resource_pool.hh"

legacy_resource_pool::container::~container() {};

legacy_resource_pool::legacy_resource_pool(context& ctx): glresource(ctx) { }
legacy_resource_pool::~legacy_resource_pool() { }

resource_pool::resource_pool(
    context& ctx,
    const std::vector<std::string>& shader_path,
    const std::optional<std::string>& shader_binary_path
): legacy_resource_pool(ctx), shader_pool(ctx, shader_path, shader_binary_path)
{
}
resource_pool::~resource_pool() {}
