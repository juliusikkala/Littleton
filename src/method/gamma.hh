#ifndef METHOD_GAMMA_HH
#define METHOD_GAMMA_HH
#include "pipeline.hh"
#include "vertex_buffer.hh"

class shader_store;
class texture;

namespace method
{
    class gamma: public target_method
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

        std::string get_name() const override;

    private:
        texture* src;

        float g;
        shader* gamma_shader;
        vertex_buffer fullscreen_quad;
    };
}
#endif

