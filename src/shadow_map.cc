#include "shadow_map.hh"

directional_shadow_map::directional_shadow_map(
    context& ctx,
    glm::uvec2 size,
    glm::vec3 position,
    directional_light* light
): render_target(ctx, size), position(position), light(light),
   depth(ctx, size, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT)
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
}

directional_shadow_map::directional_shadow_map(directional_shadow_map&& other)
: render_target(other), position(other.position), light(other.light),
  depth(std::move(other.depth))
{
    other.fbo = 0;
}

directional_shadow_map::~directional_shadow_map()
{
    if(fbo != 0) glDeleteFramebuffers(1, &fbo);
}

void directional_shadow_map::set_position(glm::vec3 position)
{
    this->position = position;
}

glm::vec3 directional_shadow_map::get_position() const
{
    return position;
}

void directional_shadow_map::set_target(glm::vec3 target_pos, float distance)
{
    if(!light) return;
    this->position = target_pos - distance * light->get_direction();
}

void directional_shadow_map::set_light(directional_light* light)
{
    this->light = light;
}

directional_light* directional_shadow_map::get_light() const
{
    return light;
}

texture& directional_shadow_map::get_depth()
{
    return depth;
}

const texture& directional_shadow_map::get_depth() const
{
    return depth;
}
