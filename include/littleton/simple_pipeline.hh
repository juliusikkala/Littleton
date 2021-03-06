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
#ifndef LT_SIMPLE_PIPELINE_HH
#define LT_SIMPLE_PIPELINE_HH
#include "api.hh"
#include "pipeline.hh"
#include "scene.hh"
#include "gbuffer.hh"
#include "doublebuffer.hh"
#include "method/apply_sg.hh"
#include "method/blit_framebuffer.hh"
#include "method/bloom.hh"
#include "method/clear.hh"
#include "method/clear_gbuffer.hh"
#include "method/draw_texture.hh"
#include "method/forward_pass.hh"
#include "method/generate_depth_mipmap.hh"
#include "method/generate_sg.hh"
#include "method/geometry_pass.hh"
#include "method/lighting_pass.hh"
#include "method/render_2d.hh"
#include "method/sao.hh"
#include "method/ssao.hh"
#include "method/shadow_msm.hh"
#include "method/shadow_pcf.hh"
#include "method/atmosphere.hh"
#include "method/sdf.hh"
#include "method/skybox.hh"
#include "method/ssao.hh"
#include "method/ssrt.hh"
#include "method/tonemap.hh"
#include "method/visualize_cubemap.hh"
#include "method/visualize_gbuffer.hh"
#include <vector>
#include <memory>
#include <string>
#include <tuple>

namespace lt
{

class resource_pool;

// Simple in usage, not in implementation :P
template<typename... Stages>
class basic_simple_pipeline: public pipeline
{
friend class simple_pipeline_builder;
public:
    using stage_ptrs = std::tuple<std::unique_ptr<Stages>...>;

protected:
    basic_simple_pipeline(
        render_target& target,
        gbuffer* buf1,
        gbuffer* buf2,
        doublebuffer* dbuf,
        std::vector<pipeline_method*>&& dynamic_stages,
        std::vector<pipeline_method*>&& static_stages,
        stage_ptrs&& all_stages,
        stage_ptrs&& overlay_stages
    );

    // Double G-Buffering! The second buffer might not be in use, depends on
    // pipeline stages. Check whether it's nullptr to make sure.
    gbuffer* buf[2];
    doublebuffer* dbuf;
    std::tuple<std::unique_ptr<Stages>...> render_stages;
    std::tuple<std::unique_ptr<Stages>...> overlay_stages;
    std::vector<pipeline_method*> static_stages;

public:
    ~basic_simple_pipeline();

    // Generates static resources such as cubemaps, environment probes and so on
    // Typically, quite costly and should be run rarely, such as when loading a
    // new scene.
    void execute_static();

    template<typename Scene>
    void set_scenes(Scene& scene);

    template<typename S>
    void set_scene(S* scene);

    template<typename Scene>
    void set_overlay_scenes(Scene& scene);

    template<typename S>
    void set_overlay_scene(S* scene);

    template<unsigned i>
    auto get_stage() -> decltype(std::get<i>(render_stages).get());

    template<unsigned i>
    auto get_overlay_stage() -> decltype(std::get<i>(overlay_stages).get());

    template<unsigned i>
    void set_options(
        const typename decltype(*std::get<i>(render_stages))::options& opt
    );

    template<unsigned i>
    void set_overlay_options(
        const typename decltype(*std::get<i>(overlay_stages))::options& opt
    );

    template<unsigned i>
    auto get_options() const ->
    const typename decltype(*std::get<i>(render_stages))::options&;

    template<unsigned i>
    auto get_overlay_options() const ->
    const typename decltype(*std::get<i>(overlay_stages))::options&;

    void update(duration delta);
};

using simple_pipeline_base = basic_simple_pipeline<
    method::clear_gbuffer,
    method::skybox,
    method::shadow_pcf,
    method::shadow_msm,
    method::geometry_pass,
    method::render_sdf,
    method::generate_depth_mipmap,
    method::lighting_pass,
    method::forward_pass,
    method::geometry_pass,
    method::lighting_pass,
    method::lighting_pass,
    method::visualize_gbuffer,
    method::render_atmosphere,
    method::bloom,
    method::tonemap,
    method::sao,
    method::ssao,
    method::apply_sg,
    method::apply_sg,
    method::ssrt,
    method::blit_framebuffer,
    method::generate_sg,
    method::render_2d
>;

class LT_API simple_pipeline: public simple_pipeline_base
{
friend class simple_pipeline_builder;
protected:
    using simple_pipeline_base::basic_simple_pipeline;
public:
    enum stages
    {
        CLEAR_GBUFFER = 0,
        SKYBOX,
        SHADOW_PCF,
        SHADOW_MSM,
        GEOMETRY_PASS,
        RENDER_SDF,
        GENERATE_DEPTH_MIPMAP,
        LIGHTING_PASS,
        FORWARD_PASS,
        GEOMETRY_TRANSPARENCY_PASS,
        LIGHTING_TRANSPARENCY_PASS,
        LIGHTING_SDF_PASS,
        VISUALIZE_GBUFFER,
        RENDER_ATMOSPHERE,
        BLOOM,
        TONEMAP,
        SAO,
        SSAO,
        APPLY_SG,
        APPLY_SG_TRANSPARENT,
        SSRT,
        BLIT_FRAMEBUFFER,
        GENERATE_SG,
        RENDER_2D
    };

    method::shadow_pcf* get_pcf() {return get_stage<SHADOW_PCF>();}
    method::shadow_msm* get_msm() {return get_stage<SHADOW_MSM>();}
    method::generate_sg* get_sg() {return get_stage<GENERATE_SG>();}
};

// Builds a simple pipeline. If you just want to create a simple_pipeline
// without dynamically selecting your stages, you don't need to use this
// directly. Use simple_pipeline::build instead.
class LT_API simple_pipeline_builder
{
public:
    simple_pipeline_builder(
        render_target& target,
        resource_pool& pool
    );

    simple_pipeline_builder(
        render_target& target,
        resource_pool& pool,
        uvec2 resolution
    );

    enum algorithm
    {
        DEFERRED = 0,
        FORWARD,
        DEFERRED_TRANSPARENT
    };

    simple_pipeline_builder& set_algorithm(algorithm a);

    simple_pipeline_builder& set_resolution(uvec2 res);
    uvec2 get_resolution() const;

    template<typename Method>
    simple_pipeline_builder& add(const typename Method::options& opt);

    template<typename Method>
    simple_pipeline_builder& add();

    template<typename Method>
    simple_pipeline_builder& add_overlay(const typename Method::options& opt);

    template<typename Method>
    simple_pipeline_builder& add_overlay();

    simple_pipeline_builder& reset();

    simple_pipeline* build();

    // Determines needed stages based on scene. If you don't want this, use
    // build() without arguments.
    template<typename Scene>
    simple_pipeline* build(Scene& scene);

    // Determines needed stages based on given scenes. The second scene is
    // drawn as an overlay and only supports 2D elements, making it a good
    // choice for GUI and HUD elements.
    template<typename Scene, typename Scene2>
    simple_pipeline* build(Scene& scene, Scene2& overlay);

private:
    render_target& target;
    resource_pool& pool;
    uvec2 resolution;

    struct method_status
    {
        bool enabled = false;
    };

    template<typename Method>
    struct options_method_status: public method_status
    {
        typename Method::options opt = {};
    };

    // Main render methods
    algorithm lighting_approach;
    method_status skybox_status;
    options_method_status<method::render_atmosphere> atmosphere_status;
    options_method_status<method::bloom> bloom_status;
    options_method_status<method::tonemap> tonemap_status;
    options_method_status<method::render_sdf> sdf_status;
    options_method_status<method::sao> sao_status;
    options_method_status<method::ssao> ssao_status;
    options_method_status<method::ssrt> ssrt_status;
    options_method_status<method::apply_sg> sg_status;
    options_method_status<method::visualize_gbuffer> visualize_status;
    options_method_status<method::render_2d> render_2d_status;

    // Overlay methods
    options_method_status<method::render_2d> overlay_render_2d_status;
};

} // namespace lt

#include "simple_pipeline.tcc"

#endif

