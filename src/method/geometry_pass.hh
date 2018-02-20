#ifndef METHOD_GEOMETRY_PASS_HH
#define METHOD_GEOMETRY_PASS_HH
#include "pipeline.hh"
#include "gbuffer.hh"
#include "shader_store.hh"
#include "scene.hh"

namespace method
{
    class geometry_pass: public target_method
    {
    public:
        geometry_pass(
            gbuffer& buf,
            shader_store& store,
            render_scene* scene
        );

        void set_scene(render_scene* scene);
        render_scene* get_scene() const;

        void execute() override;

    private:
        multishader* geometry_shader;
        render_scene* scene;

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
};
#endif
