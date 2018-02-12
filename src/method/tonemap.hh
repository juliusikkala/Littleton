#ifndef METHOD_TONEMAP_HH
#define METHOD_TONEMAP_HH
#include "pipeline.hh"
#include "texture.hh"
#include "shader_store.hh"
#include "vertex_buffer.hh"

namespace method
{
    class tonemap: public pipeline_method
    {
    public:
        tonemap(
            render_target& target,
            texture& src,
            shader_store& store,
            float exposure = 1.0f
        );

        void set_exposure(float exposure);
        float get_exposure() const;

        void execute() override;

    private:
        texture* src;
        shader* tonemap_shader;
        vertex_buffer fullscreen_quad;

        float exposure;
    };
}
#endif
