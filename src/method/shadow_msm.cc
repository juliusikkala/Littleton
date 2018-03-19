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
        shader::path{"generic.vert", "shadow/directional_msm.frag"},
        {{"VERTEX_POSITION", "0"}}
    )),
    cubemap_depth_shader(pool.get_shader(
        shader::path{"generic.vert", "shadow/omni_msm.frag", "cubemap.geom"},
        {{"VERTEX_POSITION", "0"}}
    )),
    vertical_blur_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "blur.frag"}, {{"VERTICAL", ""}}
    )),
    horizontal_blur_shader(pool.get_shader(
        shader::path{"fullscreen.vert", "blur.frag"}, {{"HORIZONTAL", ""}}
    )),
    quad(common::ensure_quad_vertex_buffer(pool)),
    moment_sampler(
       pool.get_context(),
       GL_LINEAR,
       GL_LINEAR,
       GL_CLAMP_TO_BORDER,
       0,
       glm::vec4(0.0f, 0.0, 1.0f, 0.0f)
    ),
    cubemap_moment_sampler(
        pool.get_context(),
        GL_LINEAR,
        GL_LINEAR,
        GL_CLAMP_TO_EDGE
    )
{}

shader::definition_map method::shadow_msm::get_directional_definitions() const
{
    return {
        {"SHADOW_MAPPING", "shadow/directional_msm.glsl"},
        {"DIRECTIONAL_SHADOW_MAPPING", ""}
    };
}

shader::definition_map method::shadow_msm::get_point_definitions() const
{
    return {
        {"SHADOW_MAPPING", "shadow/omni_msm.glsl"},
        {"OMNI_SHADOW_MAPPING", ""}
    };
}

void method::shadow_msm::set_shadow_map_uniforms(
    shader* s,
    unsigned& texture_index,
    directional_shadow_map* shadow_map,
    const std::string& prefix,
    const glm::mat4& pos_to_world
){
    directional_shadow_map_msm* sm =
        static_cast<directional_shadow_map_msm*>(shadow_map);

    glm::mat4 lvp = sm->get_projection() * sm->get_view();

    s->set(
        prefix + "map",
        moment_sampler.bind(sm->moments, texture_index++)
    );
    s->set(prefix + "mvp", lvp * pos_to_world);
}

void method::shadow_msm::set_shadow_map_uniforms(
    shader* s,
    unsigned& texture_index,
    omni_shadow_map* shadow_map,
    const std::string& prefix,
    const glm::mat4& pos_to_world
){
    omni_shadow_map_msm* sm = static_cast<omni_shadow_map_msm*>(shadow_map);

    s->set(
        prefix + "map",
        cubemap_moment_sampler.bind(sm->moments, texture_index++)
    );
    s->set(prefix + "far_plane", sm->get_range().y);
}

void method::shadow_msm::execute()
{
    if(!scene) return;


    const std::vector<directional_shadow_map*>* directional_shadow_maps = NULL;
    {
        const shadow_scene::directional_map& directional =
            scene->get_directional_shadows();
        auto it = directional.find(this);
        if(it != directional.end()) directional_shadow_maps = &it->second;
    }

    const std::vector<omni_shadow_map*>* omni_shadow_maps = NULL;
    {
        const shadow_scene::omni_map& omni = scene->get_omni_shadows();
        auto it = omni.find(this);
        if(it != omni.end()) omni_shadow_maps = &it->second;
    }

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glClearColor(0, 0.63, 0, 0.63);

    if(directional_shadow_maps)
    {
        ensure_render_targets(*directional_shadow_maps);
        depth_shader->bind();
        for(directional_shadow_map* sm: *directional_shadow_maps)
        {
            directional_shadow_map_msm* msm =
                static_cast<directional_shadow_map_msm*>(sm);
            // Render depth data

            render_target* target =
                msm->get_samples() == 0 ?
                (render_target*)pp_rt.get() :
                (render_target*)ms_rt[msm->get_samples()].get();

            glEnable(GL_DEPTH_TEST);

            target->bind();
            glm::uvec2 target_size = msm->moments.get_size();
            glViewport(0, 0, target_size.x, target_size.y);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 vp = msm->get_projection() * msm->get_view();

            for(object* obj: scene->get_objects())
            {
                model* mod = obj->get_model();
                if(!mod) continue;

                glm::mat4 mvp = vp * obj->get_global_transform();
                depth_shader->set("mvp", mvp);

                for(model::vertex_group& group: *mod)
                {
                    if(!group.mesh) continue;

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

    if(omni_shadow_maps)
    {
        glEnable(GL_DEPTH_TEST);

        cubemap_depth_shader->bind();

        for(omni_shadow_map* sm: *omni_shadow_maps)
        {
            omni_shadow_map_msm* msm = static_cast<omni_shadow_map_msm*>(sm);
            // Render depth data

            msm->moments_buffer.bind();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 proj = msm->get_projection();

            glm::mat4 face_vps[6] = {
                proj * msm->get_view(0), proj * msm->get_view(1),
                proj * msm->get_view(2), proj * msm->get_view(3),
                proj * msm->get_view(4), proj * msm->get_view(5)
            };
            cubemap_depth_shader->set("face_vps", 6, face_vps);
            cubemap_depth_shader->set(
                "pos", msm->get_light()->get_global_position()
            );
            cubemap_depth_shader->set("far_plane", msm->get_range().y);

            for(object* obj: scene->get_objects())
            {
                model* mod = obj->get_model();
                if(!mod) continue;

                glm::mat4 m = obj->get_global_transform();
                cubemap_depth_shader->set("m", m);
                cubemap_depth_shader->set("mvp", m);

                for(model::vertex_group& group: *mod)
                {
                    if(!group.mesh) continue;

                    group.mesh->draw();
                }
            }
        }
    }
}

std::string method::shadow_msm::get_name() const
{
    return "shadow_msm";
}

void method::shadow_msm::ensure_render_targets(
    const std::vector<directional_shadow_map*>& directional_shadow_maps
){
    std::map<unsigned, glm::uvec2> ms;
    glm::uvec2 pp;

    for(directional_shadow_map* sm: directional_shadow_maps)
    {
        directional_shadow_map_msm* msm =
            static_cast<directional_shadow_map_msm*>(sm);

        glm::uvec2& ms_size = ms[msm->get_samples()];
        ms_size = glm::max(msm->moments.get_size(), ms_size);
        pp = glm::max(msm->moments.get_size(), pp);
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
                    pair.first,
                    pair.first > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D
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
    method::shadow_msm* method,
    context& ctx,
    glm::uvec2 size,
    unsigned samples,
    unsigned radius,
    glm::vec3 offset,
    glm::vec2 area,
    glm::vec2 depth_range,
    directional_light* light
):  directional_shadow_map(method, offset, area, depth_range, light),
    moments(ctx, size, GL_RGBA16, GL_FLOAT),
    moments_buffer(ctx, size, {{GL_COLOR_ATTACHMENT0, {&moments}}}),
    samples(samples),
    radius(radius)
{
}

directional_shadow_map_msm::directional_shadow_map_msm(
    directional_shadow_map_msm&& other
):  directional_shadow_map(other),
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

omni_shadow_map_msm::omni_shadow_map_msm(
    method::shadow_msm* method,
    context& ctx,
    glm::uvec2 size,
    unsigned samples,
    float radius,
    glm::vec2 depth_range,
    point_light* light
):  omni_shadow_map(method, depth_range, light),
    moments(ctx, size, GL_RGBA16, GL_FLOAT, 0, GL_TEXTURE_CUBE_MAP),
    moments_buffer(
        ctx,
        size,
        {{GL_COLOR_ATTACHMENT0, {&moments}},
         {GL_DEPTH_ATTACHMENT, {GL_DEPTH_COMPONENT16}}},
        0, GL_TEXTURE_CUBE_MAP
    ),
    samples(samples)
{
}

omni_shadow_map_msm::omni_shadow_map_msm(omni_shadow_map_msm&& other)
:   omni_shadow_map(other),
    moments(std::move(other.moments)),
    moments_buffer(std::move(other.moments_buffer)),
    samples(other.samples)
{
}

unsigned omni_shadow_map_msm::get_samples() const
{
    return samples;
}

texture& omni_shadow_map_msm::get_moments()
{
    return moments;
}

const texture& omni_shadow_map_msm::get_moments() const
{
    return moments;
}
