#ifndef METHOD_TONEMAP_HH
#define METHOD_TONEMAP_HH
#include "pipeline.hh"
#include "vertex_buffer.hh"
#include "sampler.hh"

class texture;
class shader_pool;

namespace method
{
    class tonemap: public target_method
    {
    public:
        tonemap(
            render_target& target,
            texture& src,
            shader_pool& store,
            float exposure = 1.0f
        );

        void set_exposure(float exposure);
        float get_exposure() const;

        void execute() override;

        std::string get_name() const override;

    private:
        texture* src;
        shader* tonemap_shader;
        vertex_buffer fullscreen_quad;
        sampler color_sampler;

        float exposure;
    };
}
#endif
