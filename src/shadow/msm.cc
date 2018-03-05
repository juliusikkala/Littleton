#include "msm.hh"
#include "object.hh"
#include "model.hh"
#include "vertex_buffer.hh"
#include "scene.hh"
#include "helpers.hh"
#include "shader_store.hh"
#include "render_target.hh"

static const texture::params moment_params(
    false,
    GL_LINEAR,
    GL_CLAMP_TO_BORDER,
    0,
    glm::vec4(0.0f, 0.0, 1.0f, 0.0f)
);

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
      color(ctx, size, GL_RGBA, GL_RGBA16, GL_FLOAT, moment_params)
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

msm_impl::msm_impl(context& ctx)
: shadow_map_impl(ctx),
  quad(vertex_buffer::create_square(ctx))
{
}

void msm_impl::render(
    shader_store& store,
    const std::set<basic_shadow_map*>& shadow_maps,
    render_scene* scene
){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    ensure_render_targets(shadow_maps);

    shader* depth_shader = store.get(
        shader::path{"generic.vert", "shadow/msm.frag"},
        {{"VERTEX_POSITION", "0"}}
    );

    shader* vertical_blur_shader = store.get(
        shader::path{"fullscreen.vert", "blur.frag"},
        {{"VERTICAL", ""}}
    );

    shader* horizontal_blur_shader = store.get(
        shader::path{"fullscreen.vert", "blur.frag"},
        {{"HORIZONTAL", ""}}
    );

    for(basic_shadow_map* shadow_map: shadow_maps)
    {
        msm_shadow_map* msm = dynamic_cast<msm_shadow_map*>(shadow_map);
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

        glm::mat4 vp = shadow_map->get_projection() * shadow_map->get_view();

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
        vertical_blur_shader->set("tex", msm->moments.bind(0));
        vertical_blur_shader->set("samples", (int)(2 * radius + 1));
        quad.draw();

        msm->moments_buffer.bind();
        horizontal_blur_shader->set("tex", pp_rt->get_color().bind(0));
        horizontal_blur_shader->set("samples", (int)(2 * radius + 1));
        quad.draw();
    }
}

shader::definition_map msm_impl::get_definitions() const
{
    return {{"SHADOW_IMPLEMENTATION", "shadow/msm.glsl"}};
}

void msm_impl::set_common_uniforms(shader* s, unsigned& texture_index)
{
}

void msm_impl::set_shadow_map_uniforms(
    shader* s,
    unsigned& texture_index,
    basic_shadow_map* shadow_map,
    const std::string& prefix,
    const glm::mat4& pos_to_world
){
    msm_shadow_map* msm = dynamic_cast<msm_shadow_map*>(shadow_map);
    if(!msm) return;

    glm::mat4 lvp = shadow_map->get_projection() * shadow_map->get_view();

    s->set(prefix + "map", msm->moments.bind(texture_index++));
    s->set(prefix + "mvp", lvp * pos_to_world);
}

void msm_impl::ensure_render_targets(
    const std::set<basic_shadow_map*>& shadow_maps
){
    std::map<unsigned, glm::uvec2> ms;
    glm::uvec2 pp;

    for(basic_shadow_map* sm: shadow_maps)
    {
        msm_shadow_map* msm = dynamic_cast<msm_shadow_map*>(sm);
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

msm_shadow_map::msm_shadow_map(
    context& ctx,
    glm::uvec2 size,
    unsigned samples,
    unsigned radius
):  moments(ctx, size, GL_RGBA, GL_RGBA16, GL_FLOAT, moment_params),
    moments_buffer(ctx, size, {&moments}),
    samples(samples),
    radius(radius)
{
}

msm_shadow_map::msm_shadow_map(msm_shadow_map&& other)
:   moments(std::move(other.moments)),
    moments_buffer(std::move(other.moments_buffer)),
    samples(other.samples),
    radius(other.radius)
{
}

unsigned msm_shadow_map::get_samples() const
{
    return samples;
}

void msm_shadow_map::set_radius(unsigned radius)
{
    this->radius = radius;
}

unsigned msm_shadow_map::get_radius() const
{
    return radius;
}

texture& msm_shadow_map::get_moments()
{
    return moments;
}

const texture& msm_shadow_map::get_moments() const
{
    return moments;
}

bool msm_shadow_map::impl_is_compatible(const shadow_map_impl* impl)
{
    return dynamic_cast<const msm_impl*>(impl) != nullptr;
}

msm_impl* msm_shadow_map::create_impl() const
{
    return new msm_impl(moments.get_context());
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
):  basic_shadow_map(ctx),
    msm_shadow_map(ctx, size, samples, radius),
    directional_shadow_map(offset, area, depth_range, light)
{}

bool directional_shadow_map_msm::impl_is_compatible(const shadow_map_impl* impl)
{ return msm_shadow_map::impl_is_compatible(impl); }

msm_impl* directional_shadow_map_msm::create_impl() const
{ return msm_shadow_map::create_impl(); }

light* directional_shadow_map_msm::get_light() const
{ return directional_shadow_map::get_light(); }

glm::mat4 directional_shadow_map_msm::get_view() const
{ return directional_shadow_map::get_view(); }

glm::mat4 directional_shadow_map_msm::get_projection() const
{ return directional_shadow_map::get_projection(); }
