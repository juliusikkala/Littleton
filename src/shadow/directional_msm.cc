#include "directional_msm.hh"
#include "object.hh"
#include "model.hh"
#include "vertex_buffer.hh"
#include "scene.hh"
#include "helpers.hh"
#include "shader_pool.hh"
#include "render_target.hh"

class ms_render_target: public render_target
{
public:
    ms_render_target(context& ctx, glm::uvec2 size, unsigned samples)
    : render_target(ctx, size)
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenRenderbuffers(1, &depth_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);

        glRenderbufferStorageMultisample(
            GL_RENDERBUFFER,
            samples,
            GL_DEPTH_COMPONENT16,
            size.x,
            size.y
        );

        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER,
            depth_rbo
        );

        glGenRenderbuffers(1, &color_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, color_rbo);

        glRenderbufferStorageMultisample(
            GL_RENDERBUFFER,
            samples,
            GL_RGBA16,
            size.x,
            size.y
        );

        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_RENDERBUFFER,
            color_rbo
        );

        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error(
                "Failed to create a multisample render target for MSM"
            );

        reinstate_current_fbo();
    }

    ~ms_render_target()
    {
        if(depth_rbo != 0) glDeleteRenderbuffers(1, &depth_rbo);
        if(color_rbo != 0) glDeleteRenderbuffers(1, &color_rbo);
        if(fbo != 0) glDeleteFramebuffers(1, &fbo);
    }

private:
    GLuint depth_rbo, color_rbo;
};

class pp_render_target: public render_target
{
public:
    pp_render_target(context& ctx, glm::uvec2 size)
    : render_target(ctx, size),
      color(ctx, size, GL_RGBA16, GL_FLOAT)
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenRenderbuffers(1, &depth_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);

        glRenderbufferStorage(
            GL_RENDERBUFFER,
            GL_DEPTH_COMPONENT16,
            size.x,
            size.y
        );

        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER,
            depth_rbo
        );

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            color.get_target(),
            color.get_texture(),
            0
        );

        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error(
                "Failed to create a postprocess render target for MSM"
            );

        reinstate_current_fbo();
    }

    ~pp_render_target()
    {
        if(depth_rbo != 0) glDeleteRenderbuffers(1, &depth_rbo);
        if(fbo != 0) glDeleteFramebuffers(1, &fbo);
    }

    texture& get_color()
    {
        return color;
    }

private:
    texture color;
    GLuint depth_rbo;
};

directional_msm_impl::directional_msm_impl(context& ctx)
: directional_shadow_map_impl(ctx),
  quad(vertex_buffer::create_square(ctx)),
  moment_sampler(
    ctx,
    GL_LINEAR,
    GL_LINEAR,
    GL_CLAMP_TO_BORDER,
    0,
    glm::vec4(0.0f, 0.0, 1.0f, 0.0f)
  )
{
}

void directional_msm_impl::render(
    shader_pool& pool,
    const std::vector<directional_shadow_map*>& shadow_maps,
    render_scene* scene
){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    ensure_render_targets(shadow_maps);

    shader* depth_shader = pool.get(
        shader::path{"generic.vert", "shadow/msm.frag"},
        {{"VERTEX_POSITION", "0"}}
    );

    shader* vertical_blur_shader = pool.get(
        shader::path{"fullscreen.vert", "blur.frag"},
        {{"VERTICAL", ""}}
    );

    shader* horizontal_blur_shader = pool.get(
        shader::path{"fullscreen.vert", "blur.frag"},
        {{"HORIZONTAL", ""}}
    );

    for(directional_shadow_map* shadow_map: shadow_maps)
    {
        directional_shadow_map_msm* msm =
            dynamic_cast<directional_shadow_map_msm*>(shadow_map);
        if(!msm) continue;

        // Render depth data
        depth_shader->bind();

        render_target* target =
            msm->get_samples() == 0 ?
            (render_target*)pp_rt.get() :
            (render_target*)ms_rt[msm->get_samples()].get();

        glEnable(GL_DEPTH_TEST);

        target->bind();
        glm::uvec2 target_size = msm->moments.get_size();
        glViewport(0, 0, target_size.x, target_size.y);

        glClearColor(0, 0.63, 0, 0.63);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 vp = msm->get_projection() * msm->get_view();

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

        target->bind(GL_READ_FRAMEBUFFER);
        msm->moments_buffer.bind(GL_DRAW_FRAMEBUFFER);
        glBlitFramebuffer(
            0, 0, target_size.x, target_size.y,
            0, 0, target_size.x, target_size.y,
            GL_COLOR_BUFFER_BIT,
            GL_NEAREST
        );

        // Blur the depth (thanks to moment magic this can be done!)
        unsigned radius = msm->get_radius();
        if(radius == 0) continue;

        vertical_blur_shader->bind();

        glDisable(GL_DEPTH_TEST);

        pp_rt->bind();
        glViewport(0, 0, target_size.x, target_size.y);
        vertical_blur_shader->set(
            "tex",
            moment_sampler.bind(msm->moments, 0)
        );
        vertical_blur_shader->set("samples", (int)(2 * radius + 1));
        quad.draw();

        msm->moments_buffer.bind();
        horizontal_blur_shader->set(
            "tex",
            moment_sampler.bind(pp_rt->get_color(), 0)
        );
        horizontal_blur_shader->set("samples", (int)(2 * radius + 1));
        quad.draw();
    }
}

shader::definition_map directional_msm_impl::get_definitions() const
{
    return {{"SHADOW_MAPPING", "shadow/directional_msm.glsl"}};
}

void directional_msm_impl::set_common_uniforms(shader*, unsigned&) { }

void directional_msm_impl::set_shadow_map_uniforms(
    shader* s,
    unsigned& texture_index,
    directional_shadow_map* shadow_map,
    const std::string& prefix,
    const glm::mat4& pos_to_world
){
    directional_shadow_map_msm* msm =
        dynamic_cast<directional_shadow_map_msm*>(shadow_map);
    if(!msm) return;

    glm::mat4 lvp = msm->get_projection() * msm->get_view();

    s->set(
        prefix + "map",
        moment_sampler.bind(msm->moments, texture_index++)
    );
    s->set(prefix + "mvp", lvp * pos_to_world);
}

void directional_msm_impl::ensure_render_targets(
    const std::vector<directional_shadow_map*>& shadow_maps
){
    std::map<unsigned, glm::uvec2> ms;
    glm::uvec2 pp;

    for(directional_shadow_map* sm: shadow_maps)
    {
        directional_shadow_map_msm* msm =
            dynamic_cast<directional_shadow_map_msm*>(sm);
        if(!msm) return;

        glm::uvec2& ms_size = ms[msm->get_samples()];
        ms_size = glm::max(msm->moments.get_size(), ms_size);
        pp = glm::max(msm->moments.get_size(), pp);
    }

    for(auto& pair: ms)
    {
        std::unique_ptr<ms_render_target>& cur = ms_rt[pair.first];

        glm::uvec2 ms_size = cur ? cur->get_size() : glm::uvec2(0);

        if(ms_size.x < pair.second.x || ms_size.y < pair.second.y)
        {
            cur.reset(
                new ms_render_target(
                    get_context(),
                    glm::max(pair.second, ms_size),
                    pair.first
                )
            );
        }
    }

    glm::uvec2 pp_size = pp_rt ? pp_rt->get_size() : glm::uvec2(0);
    if(pp_size.x < pp.x || pp_size.y < pp.y)
    {
        pp_rt.reset(
            new pp_render_target(
                get_context(),
                glm::max(pp, pp_size)
            )
        );
    }
}

directional_shadow_map_msm::directional_shadow_map_msm(
    context& ctx,
    glm::uvec2 size,
    unsigned samples,
    unsigned radius,
    glm::vec3 offset,
    glm::vec2 area,
    glm::vec2 depth_range,
    directional_light* light
):  directional_shadow_map(ctx, offset, area, depth_range, light),
    moments(ctx, size, GL_RGBA16, GL_FLOAT),
    moments_buffer(ctx, size, {{GL_COLOR_ATTACHMENT0, {&moments}}}),
    samples(samples),
    radius(radius)

{}

directional_shadow_map_msm::directional_shadow_map_msm(directional_shadow_map_msm&& other)
:   directional_shadow_map(other),
    moments(std::move(other.moments)),
    moments_buffer(std::move(other.moments_buffer)),
    samples(other.samples),
    radius(other.radius)
{
}

unsigned directional_shadow_map_msm::get_samples() const
{
    return samples;
}

void directional_shadow_map_msm::set_radius(unsigned radius)
{
    this->radius = radius;
}

unsigned directional_shadow_map_msm::get_radius() const
{
    return radius;
}

texture& directional_shadow_map_msm::get_moments()
{
    return moments;
}

const texture& directional_shadow_map_msm::get_moments() const
{
    return moments;
}

bool directional_shadow_map_msm::impl_is_compatible(
    const directional_shadow_map_impl* impl
){
    return dynamic_cast<const directional_msm_impl*>(impl) != nullptr;
}

directional_msm_impl* directional_shadow_map_msm::create_impl() const
{
    return new directional_msm_impl(moments.get_context());
}
