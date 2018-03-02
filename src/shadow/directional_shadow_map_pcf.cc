#include "directional_shadow_map_pcf.hh"
#include "object.hh"
#include "scene.hh"

directional_shadow_map_pcf::directional_shadow_map_pcf(
    context& ctx,
    glm::uvec2 size,
    unsigned samples,
    float radius,
    glm::vec3 offset,
    glm::vec2 area,
    glm::vec2 depth_range,
    directional_light* light
):  directional_shadow_map(ctx, offset, area, depth_range, light),
    depth(
       ctx,
       size,
       GL_DEPTH_COMPONENT,
       GL_DEPTH_COMPONENT16,
       GL_FLOAT,
       texture::params(false, GL_LINEAR, GL_CLAMP_TO_BORDER, 0, glm::vec4(1))
    ),
    depth_buffer(ctx, size),
    radius(radius), samples(samples)
{
    depth_buffer.set_depth_target(&depth);
    set_bias();
}

directional_shadow_map_pcf::directional_shadow_map_pcf(
    directional_shadow_map_pcf&& other
):  directional_shadow_map(other),
    depth(std::move(other.depth)),
    depth_buffer(std::move(other.depth_buffer)),
    min_bias(other.min_bias), max_bias(other.max_bias),
    radius(other.radius), samples(other.samples)
{
}

directional_shadow_map_pcf::~directional_shadow_map_pcf()
{
}

void directional_shadow_map_pcf::set_bias(float min_bias, float max_bias)
{
    this->min_bias = min_bias;
    this->max_bias = max_bias;
}

glm::vec2 directional_shadow_map_pcf::get_bias() const
{
    return glm::vec2(min_bias, max_bias);
}

void directional_shadow_map_pcf::set_samples(unsigned samples)
{
    this->samples = samples;
}

unsigned directional_shadow_map_pcf::get_samples() const
{
    return samples;
}

void directional_shadow_map_pcf::set_radius(float radius)
{
    this->radius = radius;
}

float directional_shadow_map_pcf::set_radius() const
{
    return radius;
}

texture& directional_shadow_map_pcf::get_depth()
{
    return depth;
}

const texture& directional_shadow_map_pcf::get_depth() const
{
    return depth;
}

void directional_shadow_map_pcf::render(
    shader_store& store,
    render_scene* scene,
    shared_resources* resources
){
    shader* depth_shader = store.get(
        shader::path{"generic.vert", "empty.frag"},
        {{"VERTEX_POSITION", "0"}}
    );

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    depth_shader->bind();

    depth_buffer.bind();
    glClear(GL_DEPTH_BUFFER_BIT);

    glm::mat4 vp = get_projection() * get_view();

    for(object* obj: scene->get_objects())
    {
        model* mod = obj->get_model();
        if(!mod) continue;

        glm::mat4 mvp = vp * obj->get_global_transform();

        for(model::vertex_group& group: *mod)
        {
            if(!group.mesh) continue;

            depth_shader->set("mvp", mvp);
            group.mesh->draw();
        }
    }
}

bool directional_shadow_map_pcf::merge_shared_resources(
    shared_resources* res
) const
{
    pcf_shared_resources* r = dynamic_cast<pcf_shared_resources*>(res);
    if(!r) return false;

    if(r->get_kernel_size() < samples)
        r->set_kernel_size(samples);
    return true;
}

directional_shadow_map_pcf::shared_resources*
directional_shadow_map_pcf::create_shared_resources() const
{
    return new pcf_shared_resources(get_context(), samples);
}

void directional_shadow_map_pcf::set_uniforms(
    shader* s,
    const std::string& prefix,
    unsigned& texture_index,
    const glm::mat4& pos_to_world
){
    glm::mat4 lvp = get_projection() * get_view();
    glm::vec2 bias = get_bias();

    s->set(prefix + "map", depth.bind(texture_index++));
    s->set(prefix + "min_bias", bias.x);
    s->set(prefix + "max_bias", bias.y);
    s->set(prefix + "radius", radius);
    s->set(prefix + "mvp", lvp * pos_to_world);
    s->set<int>(prefix + "samples", (int)samples);
}

directional_shadow_map_pcf::pcf_shared_resources::pcf_shared_resources(
    context& ctx,
    unsigned kernel_size
):  glresource(ctx),
    kernel_size(kernel_size),
    shadow_noise(
        generate_shadow_noise_texture(
            ctx,
            glm::uvec2(512)
        )
    )
{}

void directional_shadow_map_pcf::pcf_shared_resources::set_kernel_size(
    unsigned kernel_size
){
    this->kernel_size = kernel_size;
}

unsigned
directional_shadow_map_pcf::pcf_shared_resources::get_kernel_size() const
{
    return kernel_size;
}

shader::definition_map
directional_shadow_map_pcf::pcf_shared_resources::get_definitions() const
{
    return {
        {"SHADOW_IMPLEMENTATION", "shadow/pcf.glsl"},
        {"SHADOW_MAP_KERNEL_SIZE", std::to_string(kernel_size)},
    };
}

void directional_shadow_map_pcf::pcf_shared_resources::set_uniforms(
    shader* s,
    unsigned& texture_index
){
    generate_resources();

    s->set("shadow_noise", shadow_noise->bind(texture_index++));
    s->set<glm::vec2>(
        "shadow_kernel",
        kernel_size,
        shadow_kernel.data()
    );
}

void directional_shadow_map_pcf::pcf_shared_resources::generate_resources()
{
    if(shadow_kernel.size() < kernel_size)
    {
        mitchell_best_candidate(
            shadow_kernel,
            1.0f,
            30,
            kernel_size
        );
    }
}

