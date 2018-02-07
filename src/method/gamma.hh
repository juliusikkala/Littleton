#ifndef METHOD_GAMMA_HH
#define METHOD_GAMMA_HH
#include "pipeline.hh"
#include "gbuffer.hh"
#include "shader_store.hh"
#include "vertex_buffer.hh"

namespace method
{
    class gamma: public pipeline_method
    {
    public:
        gamma(
            render_target& target,
            texture& src,
            shader_store& store,
            float gamma = 2.2
        );

        void set_gamma(float gamma);
        float get_gamma() const;

        void execute() override;

    private:
        texture* src;

        float g;
        shader* gamma_shader;
        vertex_buffer fullscreen_quad;
    };
}
#endif

