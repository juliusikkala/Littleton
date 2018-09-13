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
        gbuffer* bufs[2];
        unsigned buf_index;

        buffers(
            context& ctx,
            uvec2 size,
            bool normal,
            bool color,
            bool material,
            bool lighting,
            bool linear_depth,
            bool depth_stencil,
            bool indirect_lighting
        ):   ctx(ctx), size(size), color(color), material(material),
             lighting(lighting), linear_depth(linear_depth),
             depth_stencil(depth_stencil), indirect_lighting(indirect_lighting),
             bufs{0}, buf_index(0)
        {
        }

        gbuffer* get_in()
        {
            return init_buf(buf_index);
        }

        gbuffer* get_out()
        {
            return init_buf(buf_index^1);
        }
        
        void swap()
        {
            buf_index ^= 1;
        }

        gbuffer* init_buf(unsigned index)
        {
            if(bufs[index]) return bufs[index];
            return bufs[index] = new gbuffer(
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
                    new texture(ctx, size, GL_RGB16F, GL_FLOAT) : nullptr,
                linear_depth ?
                    new texture(ctx, size, GL_RG16F, GL_FLOAT) : nullptr,
                depth_stencil ?
                    new texture(
                        ctx, size, GL_DEPTH24_STENCIL8, GL_UNSIGNED_INT_24_8
                    ) : nullptr,
                indirect_lighting ?
                    new texture(ctx, size, GL_RGB16F, GL_FLOAT) :
                    nullptr
            );
        }
    };
}

namespace lt
{

simple_pipeline_builder::simple_pipeline_builder(
    render_target& target,
    resource_pool& pool
): target(target), pool(pool), lighting_approach(DEFERRED)
{
}

void simple_pipeline_builder::set_algorithm(algorithm a)
{
    lighting_approach = a;
}

void simple_pipeline_builder::reset()
{
    lighting_approach = DEFERRED;
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

    // Determine which buffers we need
    if(lighting_approach == DEFERRED || lighting_approach == HYBRID ||
       lighting_approach == VISUALIZER)
    {
        normal = true;
        color = true;
        material = true;
        depth_stencil = true;
    }

    if(sg_status.enabled) indirect_lighting = true;
    if(sao_status.enabled || ssrt_status.enabled) linear_depth = true;

    // Create buffers handler
    buffers b(
        pool.get_context(),
        target.get_size(),
        normal, color, material, lighting, linear_depth, depth_stencil,
        indirect_lighting
    );

    simple_pipeline::stage_ptrs stages;
    std::vector<pipeline_method*> dynamic_stages;
    std::vector<pipeline_method*> static_stages;

    // TODO: Create stages

    return new simple_pipeline(
        target,
        b.bufs[0],
        b.bufs[1],
        std::move(dynamic_stages),
        std::move(static_stages),
        std::move(stages)
    );
}

}
