#include "shadow_map.hh"
#include "helpers.hh"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

directional_shadow_map::directional_shadow_map(
    context& ctx,
    glm::uvec2 size,
    glm::vec3 offset,
    glm::vec2 area,
    glm::vec2 depth_range,
    directional_light* light
): render_target(ctx, size), up(0,1,0), light(light),
   depth(
       ctx,
       size,
       GL_DEPTH_COMPONENT,
       GL_DEPTH_COMPONENT16,
       GL_FLOAT,
       texture::SHADOW_MAP_PARAMS
   )
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        depth.get_target(),
        depth.get_texture(),
        0
    );

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Shadow map is incomplete!");

    reinstate_current_fbo();

    set_volume(area, depth_range);
    target.set_position(offset);
    set_bias();
}

directional_shadow_map::directional_shadow_map(directional_shadow_map&& other)
: render_target(other), target(other.target), up(other.up),
  projection(other.projection), light(other.light),
  depth(std::move(other.depth)), min_bias(other.min_bias),
  max_bias(other.max_bias)
{
    other.fbo = 0;
}

directional_shadow_map::~directional_shadow_map()
{
    if(fbo != 0) glDeleteFramebuffers(1, &fbo);
}

void directional_shadow_map::set_parent(transformable_node* parent)
{
    target.set_parent(parent);
}

void directional_shadow_map::set_offset(glm::vec3 offset)
{
    target.set_position(offset);
}

glm::vec3 directional_shadow_map::get_offset() const
{
    return target.get_position();
}

void directional_shadow_map::set_volume(
    glm::vec2 area,
    glm::vec2 depth_range
){
    projection = glm::ortho(
        -area.x/2,
        area.x/2,
        -area.y/2,
        area.y/2,
        depth_range.x,
        depth_range.y
    );
}

void directional_shadow_map::set_up_axis(glm::vec3 up)
{
    this->up = up;
}

glm::mat4 directional_shadow_map::get_view() const
{
    if(!light) return glm::mat4(0);

    glm::mat4 translation = glm::translate(-target.get_global_position());
    glm::mat4 rotation = glm::mat4(
        glm::inverse(quat_lookat(light->get_direction(), up))
    );
    return rotation * translation;
}

glm::mat4 directional_shadow_map::get_projection() const
{
    return projection;
}

void directional_shadow_map::set_light(directional_light* light)
{
    this->light = light;
}

directional_light* directional_shadow_map::get_light() const
{
    return light;
}

void directional_shadow_map::set_bias(float min_bias, float max_bias)
{
    this->min_bias = min_bias;
    this->max_bias = max_bias;
}

glm::vec2 directional_shadow_map::get_bias() const
{
    return glm::vec2(min_bias, max_bias);
}

texture& directional_shadow_map::get_depth()
{
    return depth;
}

const texture& directional_shadow_map::get_depth() const
{
    return depth;
}

texture* generate_shadow_noise_texture(context& ctx, glm::uvec2 size)
{
    // Just generate sines and cosines to rotate the kernel by.
    std::vector<glm::vec2> shadow_samples;
    shadow_samples.resize(size.x * size.y);

    for(unsigned i = 0; i < size.x * size.y; ++i)
    {
        shadow_samples[i] = glm::circularRand(1.0f);
    }

    return new texture(
        ctx,
        size,
        GL_RG,
        GL_RG8_SNORM,
        GL_FLOAT,
        {false, GL_LINEAR, GL_REPEAT, 0},
        GL_TEXTURE_2D,
        (float*)shadow_samples.data()
    );
}
