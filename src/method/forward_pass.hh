#ifndef METHOD_FORWARD_PASS_HH
#define METHOD_FORWARD_PASS_HH
#include "pipeline.hh"
#include "shader_store.hh"
#include "scene.hh"

namespace method
{
    class forward_pass: public target_method
    {
    public:
        forward_pass(
            render_target& target,
            shader_store& shaders,
            render_scene* scene = nullptr
        );
        ~forward_pass();

        void set_shader(multishader* s);
        multishader* get_shader() const;

        void set_scene(render_scene* s);
        render_scene* get_scene() const;

        void set_shadow(unsigned samples = 16, float radius = 4.0f);
        
        void execute() override;

    private:
        multishader* forward_shader;
        render_scene* scene;

        std::unique_ptr<texture> shadow_noise;
        std::vector<glm::vec2> shadow_kernel;

        // It doesn't matter if a vertex group is reallocated; definitions
        // will get updated anyways. However, this avoids malloc() calls by
        // storing the definitions (which _should_ change rarely). Thanks to
        // update_definitions(), no allocs are actually made if all values are
        // correct.
        std::map<
            model::vertex_group*,
            shader::definition_map
        > definitions_cache;
    };
}

#endif
