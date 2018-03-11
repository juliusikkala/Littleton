#include "shadow_msm.hh"
#include "resource_pool.hh"
#include "helpers.hh"
#include "object.hh"
#include "camera.hh"
#include "scene.hh"
#include "common_resources.hh"

method::shadow_msm::shadow_msm(resource_pool& pool, render_scene* scene)
:   glresource(pool.get_context()),
    shadow_method(scene),
    depth_shader(pool.get_shader(
        shader::path{"generic.vert", "shadow/msm.frag"},
        {{"VERTEX_POSITION", "0"}}
    )),
    vertical_blur_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "blur.frag"},
        {{"VERTICAL", ""}}
    )),
    horizontal_blur_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "blur.frag"},
        {{"HORIZONTAL", ""}}
    )),
    quad(common::ensure_quad_vertex_buffer(pool)),
    moment_sampler(
       pool.get_context(),
       GL_LINEAR,
       GL_LINEAR,
       GL_CLAMP_TO_BORDER,
       0,
       glm::vec4(0.0f, 0.0, 1.0f, 0.0f)
    )
{}

void method::shadow_msm::add(directional_shadow_map_msm* shadow_map)
{
    sorted_insert(directional_shadow_maps, shadow_map);
}

void method::shadow_msm::remove(directional_shadow_map_msm* shadow_map)
{
    sorted_erase(directional_shadow_maps, shadow_map);
}

void method::shadow_msm::clear()
{
    directional_shadow_maps.clear();
}

shader::definition_map method::shadow_msm::get_directional_definitions() const
{
    return {{"SHADOW_MAPPING", "shadow/directional_msm.glsl"}};
}

size_t method::shadow_msm::get_directional_shadow_map_count() const
{
    return directional_shadow_maps.size();
}

directional_shadow_map_msm*
method::shadow_msm::get_directional_shadow_map(unsigned i) const
{
    return directional_shadow_maps[i];
}

void method::shadow_msm::set_directional_shadow_map_uniforms(
    shader* s,
    unsigned& texture_index,
    unsigned i,
    const std::string& prefix,
    const glm::mat4& pos_to_world
){
    directional_shadow_map_msm* sm = directional_shadow_maps[i];

    glm::mat4 lvp = sm->get_projection() * sm->get_view();

    s->set(
        prefix + "map",
        moment_sampler.bind(sm->moments, texture_index++)
    );
    s->set(prefix + "mvp", lvp * pos_to_world);
}

void method::shadow_msm::execute()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    ensure_render_targets();

    for(directional_shadow_map_msm* msm: directional_shadow_maps)
    {
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
            moment_sampler.bind(
                *pp_rt->get_texture_target(GL_COLOR_ATTACHMENT0), 0
            )
        );
        horizontal_blur_shader->set("samples", (int)(2 * radius + 1));
        quad.draw();
    }
}

std::string method::shadow_msm::get_name() const
{
    return "shadow_msm";
}

void method::shadow_msm::ensure_render_targets()
{
    std::map<unsigned, glm::uvec2> ms;
    glm::uvec2 pp;

    for(directional_shadow_map_msm* sm: directional_shadow_maps)
    {
        glm::uvec2& ms_size = ms[sm->get_samples()];
        ms_size = glm::max(sm->moments.get_size(), ms_size);
        pp = glm::max(sm->moments.get_size(), pp);
    }

    for(auto& pair: ms)
    {
        std::unique_ptr<framebuffer>& cur = ms_rt[pair.first];

        glm::uvec2 ms_size = cur ? cur->get_size() : glm::uvec2(0);

        if(ms_size.x < pair.second.x || ms_size.y < pair.second.y)
        {
            cur.reset(
                new framebuffer(
                    get_context(),
                    glm::max(pair.second, ms_size),
                    {{GL_DEPTH_ATTACHMENT, {GL_DEPTH_COMPONENT16}},
                     {GL_COLOR_ATTACHMENT0, {GL_RGBA16}}},
                    pair.first
                )
            );
        }
    }

    glm::uvec2 pp_size = pp_rt ? pp_rt->get_size() : glm::uvec2(0);
    if(pp_size.x < pp.x || pp_size.y < pp.y)
    {
        pp_rt.reset(
            new framebuffer(
                get_context(),
                glm::max(pp, pp_size),
                {{GL_DEPTH_ATTACHMENT, {GL_DEPTH_COMPONENT16}},
                 {GL_COLOR_ATTACHMENT0, {GL_RGBA16, true}}}
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
):  directional_shadow_map(offset, area, depth_range, light),
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
