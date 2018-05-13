#include "resource_pool.hh"

namespace lt
{

resource_pool::resource_pool(
    context& ctx,
    const std::vector<std::string>& shader_path,
    const std::optional<std::string>& shader_binary_path
):  glresource(ctx), shader_pool(ctx, shader_path, shader_binary_path),
    texture_pool(ctx), gpu_buffer_pool(ctx), primitive_pool(ctx),
    sampler_pool(ctx), material_pool(ctx), model_pool(ctx),
    framebuffer_pool(ctx)
{
}
resource_pool::~resource_pool() {}

framebuffer_pool::loaner resource_pool::loan_framebuffer(
    glm::uvec2 size,
    const framebuffer::target_specification_map& target_specifications,
    unsigned samples
){ return framebuffer_pool::loan(size, target_specifications, samples); }

multishader* resource_pool::add_shader(const shader::path& path)
{
    return shader_pool::add(path);
}

void resource_pool::remove_shader(const shader::path& path)
{
    shader_pool::remove(path);
}

multishader* resource_pool::get_shader(const shader::path& path)
{
    return shader_pool::get(path);
}

shader* resource_pool::get_shader(
    const shader::path& path,
    const shader::definition_map& definitions
){
    return shader_pool::get(path, definitions);
}

#define generic_resource_alias_impl(type, base) \
type* resource_pool::add_ ## type (const std::string& name, type* t, bool id) \
{ return base ::add(name, t, id); } \
type* resource_pool::add_ ## type (const std::string& name, type&& t, bool id) \
{ return base ::add(name, std::move(t), id); } \
void resource_pool::remove_ ## type(const std::string& name) \
{ base ::remove(name); } \
const type* resource_pool::get_ ## type(const std::string& name) \
{ return base ::get(name); } \
bool resource_pool::contains_ ## type(const std::string& name) \
{ return base ::contains(name); }

generic_resource_alias_impl(texture, texture_pool);
generic_resource_alias_impl(gpu_buffer, gpu_buffer_pool);
generic_resource_alias_impl(primitive, primitive_pool);
generic_resource_alias_impl(sampler, sampler_pool);
generic_resource_alias_impl(material, material_pool);
generic_resource_alias_impl(model, model_pool);

void resource_pool::load_all()
{
    texture_pool::load_all();
    gpu_buffer_pool::load_all();
    primitive_pool::load_all();
}

void resource_pool::unload_all()
{
    shader_pool::unload_all();
    framebuffer_pool::unload_all();
    texture_pool::unload_all();
    gpu_buffer_pool::unload_all();
    primitive_pool::unload_all();
}

} // namespace lt
