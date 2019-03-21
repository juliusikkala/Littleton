/*
    Copyright 2018-2019 Julius Ikkala

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
#include <stdexcept>
#include <utility>
#include <type_traits>

namespace lt
{

template<typename... Stages>
basic_simple_pipeline<Stages...>::basic_simple_pipeline(
    render_target& target,
    gbuffer* buf1,
    gbuffer* buf2,
    doublebuffer* dbuf,
    std::vector<pipeline_method*>&& dynamic_stages,
    std::vector<pipeline_method*>&& static_stages,
    stage_ptrs&& render_stages,
    stage_ptrs&& overlay_stages
):  pipeline(dynamic_stages), buf{buf1, buf2}, dbuf(dbuf),
    render_stages(std::move(render_stages)),
    overlay_stages(std::move(overlay_stages)),
    static_stages(std::move(static_stages))
{
}

template<typename... Stages>
basic_simple_pipeline<Stages...>::~basic_simple_pipeline()
{
    for(gbuffer* b: buf)
    {
        if(!b) continue;
        texture* textures[] = {
            b->get_normal(),
            b->get_color(),
            b->get_material(),
            b->get_linear_depth(),
            dbuf ? nullptr : b->get_lighting(),
            b->get_depth_stencil(),
            b->get_indirect_lighting()
        };
        for(texture* t: textures) if(t) delete t;
        delete b;
    }
    delete dbuf;
}

template<typename... Stages>
void basic_simple_pipeline<Stages...>::execute_static()
{
    for(unsigned i = 0; i < static_stages.size(); ++i)
    {
        pipeline_method* method = static_stages[i];
        method->execute();
        if(glGetError() != GL_NO_ERROR)
            throw std::runtime_error(
                "Error in pipeline method "
                + method->get_name() + " index " + std::to_string(i)
            );
    }
}

template<typename T, typename=void>
struct has_set_scenes: std::false_type { };

template<typename T>
struct has_set_scenes<
    T,
    decltype((void) std::declval<T>().set_scenes({}), void())
> : std::true_type { };

template<typename... Stages>
template<typename Scene>
void basic_simple_pipeline<Stages...>::set_scenes(Scene& scene)
{
    // Calls set_scenes for all methods in render_stages if it exists for the
    // method.
    std::apply(
        [&](auto&... stages)
        {
            auto func = [&](auto& stage)
            {
                if constexpr(has_set_scenes<decltype(*stage)>::value)
                {
                    if(stage) stage->set_scenes(&scene);
                }
                return 0;
            };
            std::make_tuple(func(stages)...);
        },
        render_stages
    );
}

template<typename T, typename U, typename=void>
struct has_set_scene: std::false_type { };

template<typename T, typename U>
struct has_set_scene<
    T, U,
    decltype((void) std::declval<T>().template set_scene<U*>(nullptr), void())
> : std::true_type { };

template<typename... Stages>
template<typename S>
void basic_simple_pipeline<Stages...>::set_scene(S* scene)
{
    std::apply(
        [&](auto&... stages)
        {
            auto func = [&](auto& stage)
            {
                if constexpr(has_set_scene<decltype(*stage), S>::value)
                {
                    if(stage) stage->set_scene(scene);
                }
                return 0;
            };
            std::make_tuple(func(stages)...);
        },
        render_stages
    );
}

template<typename... Stages>
template<typename Scene>
void basic_simple_pipeline<Stages...>::set_overlay_scenes(Scene& scene)
{
    // Calls set_scenes for all methods in render_stages if it exists for the
    // method.
    std::apply(
        [&](auto&... stages)
        {
            auto func = [&](auto& stage)
            {
                if constexpr(has_set_scenes<decltype(*stage)>::value)
                {
                    if(stage) stage->set_scenes(&scene);
                }
                return 0;
            };
            std::make_tuple(func(stages)...);
        },
        overlay_stages
    );
}

template<typename... Stages>
template<typename S>
void basic_simple_pipeline<Stages...>::set_overlay_scene(S* scene)
{
    std::apply(
        [&](auto&... stages)
        {
            auto func = [&](auto& stage)
            {
                if constexpr(has_set_scene<decltype(*stage), S>::value)
                {
                    if(stage) stage->set_scene(scene);
                }
                return 0;
            };
            std::make_tuple(func(stages)...);
        },
        overlay_stages
    );
}

template<typename... Stages>
template<unsigned i>
auto basic_simple_pipeline<Stages...>::get_stage()
-> decltype(std::get<i>(render_stages).get())
{
    return std::get<i>(render_stages).get();
}

template<typename... Stages>
template<unsigned i>
auto basic_simple_pipeline<Stages...>::get_overlay_stage()
-> decltype(std::get<i>(overlay_stages).get())
{
    return std::get<i>(overlay_stages).get();
}

template<typename... Stages>
template<unsigned i>
void basic_simple_pipeline<Stages...>::set_options(
    const typename decltype(*std::get<i>(render_stages))::options& opt
){
    auto stage = std::get<i>(render_stages);
    if(stage) stage->set_options(opt);
}

template<typename... Stages>
template<unsigned i>
void basic_simple_pipeline<Stages...>::set_overlay_options(
    const typename decltype(*std::get<i>(overlay_stages))::options& opt
){
    auto stage = std::get<i>(overlay_stages);
    if(stage) stage->set_options(opt);
}

template<typename... Stages>
template<unsigned i>
auto basic_simple_pipeline<Stages...>::get_options() const
-> const typename decltype(*std::get<i>(render_stages))::options&
{
    auto stage = std::get<i>(render_stages);
    if(stage) return stage->get_options();
    throw std::runtime_error(
        "Stage " + std::to_string(i) + " not present in pipeline"
    );
}

template<typename... Stages>
template<unsigned i>
auto basic_simple_pipeline<Stages...>::get_overlay_options() const
-> const typename decltype(*std::get<i>(overlay_stages))::options&
{
    auto stage = std::get<i>(overlay_stages);
    if(stage) return stage->get_options();
    throw std::runtime_error(
        "Overlay stage " + std::to_string(i) + " not present in pipeline"
    );
}

template<typename... Stages>
void basic_simple_pipeline<Stages...>::update(duration delta)
{
    std::apply(
        [&](auto&... stages)
        {
            auto func = [&](auto& stage)
            {
                if constexpr(
                    std::is_base_of_v<
                        animated,
                        std::decay_t<decltype(*stage)>
                    >
                ) if(stage) stage->animation_update(delta);
                return 0;
            };
            std::make_tuple(func(stages)...);
        },
        render_stages
    );

    std::apply(
        [&](auto&... stages)
        {
            auto func = [&](auto& stage)
            {
                if constexpr(
                    std::is_base_of_v<
                        animated,
                        std::decay_t<decltype(*stage)>
                    >
                ) if(stage) stage->animation_update(delta);
                return 0;
            };
            std::make_tuple(func(stages)...);
        },
        overlay_stages
    );
}

template<typename Method>
simple_pipeline_builder&
simple_pipeline_builder::add(const typename Method::options& opt)
{
#define LT_HANDLE_METHOD(name, status) \
    if constexpr(std::is_same_v<Method, method:: name>) \
    { \
        status.enabled = true; \
        status.opt = opt; \
    }
    LT_HANDLE_METHOD(render_atmosphere, atmosphere_status)
    LT_HANDLE_METHOD(bloom, bloom_status)
    LT_HANDLE_METHOD(tonemap, tonemap_status)
    LT_HANDLE_METHOD(render_sdf, sdf_status)
    LT_HANDLE_METHOD(sao, sao_status)
    LT_HANDLE_METHOD(ssao, ssao_status)
    LT_HANDLE_METHOD(ssrt, ssrt_status)
    LT_HANDLE_METHOD(apply_sg, sg_status)
    LT_HANDLE_METHOD(visualize_gbuffer, visualize_status)
    LT_HANDLE_METHOD(render_2d, render_2d_status)
#undef LT_HANDLE_METHOD
    return *this;
}

template<typename Method>
simple_pipeline_builder& simple_pipeline_builder::add()
{
    if constexpr(std::is_same_v<Method, method::skybox>)
        skybox_status.enabled = true;
    else if constexpr(std::is_same_v<Method, method::render_2d>)
    {
        method::render_2d::options opt;
        opt.read_depth_buffer = true;
        opt.write_buffer_data = true;
        opt.fullbright = false;
        return add<Method>(opt);
    }
    else return add<Method>({});
    return *this;
}

template<typename Method>
simple_pipeline_builder&
simple_pipeline_builder::add_overlay(const typename Method::options& opt)
{
#define LT_HANDLE_METHOD(name, status) \
    if constexpr(std::is_same_v<Method, method:: name>) \
    { \
        status.enabled = true; \
        status.opt = opt; \
    }
    LT_HANDLE_METHOD(render_2d, overlay_render_2d_status)
#undef LT_HANDLE_METHOD
    return *this;
}

template<typename Method>
simple_pipeline_builder& simple_pipeline_builder::add_overlay()
{
    return add_overlay<Method>({});
}

template<typename Scene>
simple_pipeline* simple_pipeline_builder::build(Scene& scene)
{
    if(
        std::is_convertible_v<Scene&, environment_scene&> &&
        !skybox_status.enabled
    ) add<method::skybox>();
    if(
        std::is_convertible_v<Scene&, atmosphere_scene&> &&
        !atmosphere_status.enabled
    ) add<method::render_atmosphere>();
    if(
        std::is_convertible_v<Scene&, sdf_scene&> &&
        !sdf_status.enabled
    ) add<method::render_sdf>();
    if(
        std::is_convertible_v<Scene&, sprite_scene&> &&
        !render_2d_status.enabled
    ) add<method::render_2d>();
    simple_pipeline* res = build();
    res->set_scenes(scene);
    return res;
}

template<typename Scene, typename Scene2>
simple_pipeline* simple_pipeline_builder::build(Scene& scene, Scene2& overlay)
{
    if(
        std::is_convertible_v<Scene2&, sprite_scene&> &&
        !overlay_render_2d_status.enabled
    ) add_overlay<method::render_2d>();
    return build(scene);
}

}
