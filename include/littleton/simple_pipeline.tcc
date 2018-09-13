#include <stdexcept>
#include <utility>

namespace lt
{

template<typename... Stages>
basic_simple_pipeline<Stages...>::basic_simple_pipeline(
    render_target& target,
    gbuffer* buf1,
    gbuffer* buf2,
    std::vector<pipeline_method*>&& dynamic_stages,
    std::vector<pipeline_method*>&& static_stages,
    stage_ptrs&& all_stages
):  target_method(target), buf{buf1, buf2}, all_stages(std::move(all_stages)),
    dynamic_stages(std::move(dynamic_stages)),
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
            b->get_lighting(),
            b->get_depth_stencil(),
            b->get_indirect_lighting()
        };
        for(texture* t: textures) if(t) delete t;
        delete b;
    }
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

template<typename... Stages>
void basic_simple_pipeline<Stages...>::execute()
{
    for(unsigned i = 0; i < dynamic_stages.size(); ++i)
    {
        pipeline_method* method = dynamic_stages[i];
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
    decltype((void) std::declval<T>().set_scenes(), void())
> : std::true_type { };

template<typename... Stages>
template<typename Scene>
void basic_simple_pipeline<Stages...>::set_scenes(const Scene& scene)
{
    // Calls set_scenes for all methods in all_stages if it exists for the
    // method.
    std::apply(
        [&](auto&... stages)
        {
            auto func = [&](auto& stage)
            {
                if constexpr(has_set_scenes<decltype(*stage)>::value && stage)
                    stage->set_scenes(scene);
                return 0;
            };
            std::make_tuple(func(stages)...);
        },
        all_stages
    );
}

template<typename T, typename U, typename=void>
struct has_set_scene: std::false_type { };

template<typename T, typename U>
struct has_set_scene<
    T,
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
                if constexpr(has_set_scene<decltype(*stage), S>::value && stage)
                    stage->set_scene(scene);
                return 0;
            };
            std::make_tuple(func(stages)...);
        },
        all_stages
    );
}

template<typename... Stages>
template<unsigned i>
auto basic_simple_pipeline<Stages...>::get_stage()
-> decltype(std::get<i>(all_stages).get())
{
    return std::get<i>(all_stages).get();
}

template<typename... Stages>
template<unsigned i>
void basic_simple_pipeline<Stages...>::set_options(
    const typename decltype(*std::get<i>(all_stages))::options& opt
){
    auto stage = std::get<i>(all_stages);
    if(stage) stage->set_options(opt);
}

template<typename... Stages>
template<unsigned i>
auto basic_simple_pipeline<Stages...>::get_options() const
-> const typename decltype(*std::get<i>(all_stages))::options&
{
    auto stage = std::get<i>(all_stages);
    if(stage) return stage->get_options();
    throw std::runtime_error("Stage " + std::to_string(i) + " not present in pipeline");
}

template<typename Method>
void simple_pipeline_builder::add(typename Method::options& opt)
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
    LT_HANDLE_METHOD(sao, sao_status)
    LT_HANDLE_METHOD(ssao, ssao_status)
    LT_HANDLE_METHOD(ssrt, ssrt_status)
    LT_HANDLE_METHOD(apply_sg, sg_status)
#undef LT_HANDLE_METHOD
}

template<typename Method>
void simple_pipeline_builder::add()
{
    if constexpr(std::is_same_v<Method, method::skybox>)
        skybox_status.enabled = true;
    else add<Method>({});
}

template<typename Scene>
simple_pipeline* simple_pipeline_builder::build(const Scene& scene)
{
    // TODO: Figure out which scene types the compound scene has, then enable
    // the correct methods and call build().
    simple_pipeline* res = build();
    res->set_scenes(scene);
    return res;
}

}
