/*
    Copyright 2018 Julius Ikkala

    This file is part of Littleton.

    Littleton is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Littleton is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Littleton.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "simple_pipeline.hh"
#include "resource_pool.hh"

namespace
{
    using namespace lt;

    struct buffers
    {
        context& ctx;
        uvec2 size;
        bool normal;
        bool color;
        bool material;
        bool lighting;
        bool linear_depth;
        bool depth_stencil;
        bool indirect_lighting;
        std::unique_ptr<gbuffer> bufs[2];
        unsigned buf_index;
        doublebuffer* dbuf;

        buffers(
            context& ctx,
            uvec2 size,
            bool normal,
            bool color,
            bool material,
            bool lighting,
            bool linear_depth,
            bool depth_stencil,
            bool indirect_lighting,
            doublebuffer* dbuf
        ):   ctx(ctx), size(size), normal(normal), color(color),
             material(material), lighting(lighting), linear_depth(linear_depth),
             depth_stencil(depth_stencil), indirect_lighting(indirect_lighting),
             buf_index(0), dbuf(dbuf)
        {
        }

        gbuffer& in()
        {
            return *init_buf(buf_index);
        }

        gbuffer& out()
        {
            return *init_buf(buf_index^1);
        }
        
        void swap()
        {
            buf_index ^= 1;
        }

        gbuffer* init_buf(unsigned index)
        {
            if(!bufs[index]) bufs[index].reset(new gbuffer(
                ctx,
                size,
                normal ?
                    new texture(ctx, size, GL_RG16_SNORM, GL_UNSIGNED_BYTE) :
                    nullptr,
                color ?
                    new texture(ctx, size, GL_RGB8, GL_UNSIGNED_BYTE) : nullptr,
                material ?
                    new texture(ctx, size, GL_RGBA8, GL_UNSIGNED_BYTE) :
                    nullptr,
                lighting ?
                    (dbuf ? 
                        &dbuf->get_texture(index) :
                        new texture(ctx, size, GL_RGB16F, GL_FLOAT)):
                    nullptr,
                linear_depth ?
                    new texture(ctx, size, GL_RG16F, GL_FLOAT) : nullptr,
                depth_stencil ?
                    new texture(
                        ctx, size, GL_DEPTH24_STENCIL8, GL_UNSIGNED_INT_24_8
                    ) : nullptr,
                indirect_lighting ?
                    new texture(ctx, size, GL_RGB16F, GL_FLOAT) :
                    nullptr
            ));
            return bufs[index].get();
        }

        void sync_dbuf()
        {
            if(!dbuf) return;
            if(in().get_lighting() != &dbuf->output()) dbuf->swap();
        }
    };
}

namespace lt
{

simple_pipeline_builder::simple_pipeline_builder(
    render_target& target,
    resource_pool& pool
):  simple_pipeline_builder(target, pool, target.get_size()) {}

simple_pipeline_builder::simple_pipeline_builder(
    render_target& target,
    resource_pool& pool,
    uvec2 resolution
):  target(target), pool(pool), resolution(resolution),
    lighting_approach(DEFERRED)
{
}

void simple_pipeline_builder::set_algorithm(algorithm a)
{
    lighting_approach = a;
}

void simple_pipeline_builder::set_resolution(uvec2 res)
{
    this->resolution = res;
}

uvec2 simple_pipeline_builder::get_resolution() const
{
    return resolution;
}

void simple_pipeline_builder::reset()
{
    lighting_approach = DEFERRED;
    resolution = target.get_size();
    skybox_status.enabled = false;
    atmosphere_status.enabled = false;
    bloom_status.enabled = false;
    tonemap_status.enabled = false;
    sao_status.enabled = false;
    ssao_status.enabled = false;
    ssrt_status.enabled = false;
    sg_status.enabled = false;
}

simple_pipeline* simple_pipeline_builder::build()
{
    bool normal = false;
    bool color = false;
    bool material = false;
    bool lighting = true;
    bool linear_depth = false;
    bool depth_stencil = false;
    bool indirect_lighting = false;

    bool deferred = lighting_approach == DEFERRED ||
       lighting_approach == HYBRID || lighting_approach == VISUALIZER;
    bool postprocessed = tonemap_status.enabled || bloom_status.enabled;
    bool defer_ambient = false;

    std::unique_ptr<doublebuffer> dbuf;

    // Determine which buffers we need
    if(deferred)
    {
        normal = true;
        color = true;
        material = true;
        depth_stencil = true;
    }

    if(sg_status.enabled)
    {
        indirect_lighting = true;
        defer_ambient = true;
    }

    if(sao_status.enabled || ssrt_status.enabled)
    {
        linear_depth = true;
        defer_ambient = true;
    }

    if(postprocessed)
        dbuf.reset(
            new doublebuffer(pool.get_context(), resolution, GL_RGB16F)
        );

    // Create buffers handler
    buffers b(
        pool.get_context(),
        resolution,
        normal, color, material, lighting, linear_depth, depth_stencil,
        indirect_lighting,
        dbuf.get()
    );

    simple_pipeline::stage_ptrs stages;
    std::vector<pipeline_method*> dynamic_stages;
    std::vector<pipeline_method*> static_stages;

#define add_stage(stage_name, value, type) \
    { \
        auto* v = (value); \
        type##_stages.push_back(v); \
        std::get<(int) simple_pipeline:: stage_name >(stages).reset(v); \
    }

    // We'll always have to clear the G-Buffer
    add_stage(CLEAR_GBUFFER, new method::clear_gbuffer(b.in()), dynamic);

    // Always add shadow mapping methods since they're essentially free if there
    // are no shadows and require no configuration.
    add_stage(SHADOW_PCF, new method::shadow_pcf(pool, {}), dynamic);
    add_stage(SHADOW_MSM, new method::shadow_msm(pool, {}), dynamic);

    if(deferred)
    {
        method::geometry_pass::options gp_opt;
        gp_opt.apply_ambient = !defer_ambient;
        add_stage(
            GEOMETRY_PASS,
            new method::geometry_pass(b.in(), pool, {}, gp_opt),
            dynamic
        );
    }

    if(skybox_status.enabled)
        add_stage(SKYBOX, new method::skybox(b.in(), pool, {}), dynamic);

    if(deferred)
    {
        add_stage(
            LIGHTING_PASS,
            new method::lighting_pass(b.in(), b.in(), pool, {}),
            dynamic
        );
    }
    else
    {
        method::forward_pass::options fp_opt;
        fp_opt.apply_ambient = !defer_ambient;
        add_stage(
            FORWARD_PASS,
            new method::forward_pass(b.in(), pool, {}, fp_opt),
            dynamic
        );
    }

    if(linear_depth)
    {
        add_stage(
            GENERATE_DEPTH_MIPMAP,
            new method::generate_depth_mipmap(b.in(), pool),
            dynamic
        );
    }

    if(sg_status.enabled)
    {
        add_stage(GENERATE_SG, new method::generate_sg(pool, {}), static);
        add_stage(
            APPLY_SG,
            new method::apply_sg(b.in(), b.in(), pool, {}, sg_status.opt),
            dynamic
        );
    }

    if(sao_status.enabled)
    {
        add_stage(
            SAO,
            new method::sao(b.in(), b.in(), pool, {}, sao_status.opt),
            dynamic
        );
    }

    if(ssao_status.enabled)
    {
        add_stage(
            SSAO,
            new method::ssao(b.in(), b.in(), pool, {}, ssao_status.opt),
            dynamic
        );
    }

    if(ssrt_status.enabled)
    {
        add_stage(
            SSRT,
            new method::ssrt(b.in(), b.in(), pool, {}, ssrt_status.opt),
            dynamic
        );
    }

    if(atmosphere_status.enabled)
    {
        add_stage(
            RENDER_ATMOSPHERE,
            new method::render_atmosphere(
                b.in(),
                pool,
                {},
                b.in().get_depth_stencil(),
                atmosphere_status.opt
            ),
            dynamic
        );
    }

    // TODO: Transparency handling with SG:
    // Write 1 in first geometry/forward pass to stencil (default?)
    // First SG application (1<<7)
    // Transparency pass (write 2)
    // Second SG application (1)

    // Start postprocessing
    b.sync_dbuf();

    if(bloom_status.enabled)
    {
        add_stage(
            BLOOM,
            new method::bloom(
                dbuf->input(),
                pool,
                &dbuf->output(),
                bloom_status.opt
            ),
            dynamic
        );
        dbuf->swap();
    }

    if(tonemap_status.enabled)
    {
        add_stage(
            TONEMAP,
            new method::tonemap(
                dbuf->input(),
                pool,
                &dbuf->output(),
                tonemap_status.opt
            ),
            dynamic
        );
        dbuf->swap();
    }

    // Final blit
    if(lighting_approach == VISUALIZER)
    {
        add_stage(
            VISUALIZE_GBUFFER,
            new method::visualize_gbuffer(
                target, b.in(), pool, {},
                {
                    method::visualize_gbuffer::options::POSITION,
                    method::visualize_gbuffer::options::MATERIAL,
                    method::visualize_gbuffer::options::LIGHTING,
                    method::visualize_gbuffer::options::INDIRECT_LIGHTING
                }
            ),
            dynamic
        );
    }
    else
    {
        add_stage(
            BLIT_FRAMEBUFFER,
            new method::blit_framebuffer(
                target,
                postprocessed ?
                    (render_target&)dbuf->input(1) :
                    (render_target&)b.in(),
                method::blit_framebuffer::COLOR_ONLY
            ),
            dynamic
        );
    }

#undef add_stage

    return new simple_pipeline(
        target,
        b.bufs[0].release(),
        b.bufs[1].release(),
        dbuf.release(),
        std::move(dynamic_stages),
        std::move(static_stages),
        std::move(stages)
    );
}

}
